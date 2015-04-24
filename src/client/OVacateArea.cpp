/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2015 Richard Dijk <microvirus.multiplying@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//Filename    : OVacateArea.cpp
//Description : Helper class/function to order units to vacate a specific area.
//              Used for constructing buildings and settling towns, when a unit is blocking
//              the build spot Vacate Area can order offending units out of the way.

#include <OVacateArea.h>
#include <cstdlib>
#include <new> // nothrow
#include <algorithm> // min, max

#include <OWORLD.h>
#include <OUNIT.h>

#include <cassert>
#include <dbglog.h>

// Used by Unit exclusively, so use that channel.
DBGLOG_DEFAULT_CHANNEL(Unit);

// Set typical firm width and height to that of a town; this should work for all buildings and towns.
enum {
	TYPICAL_FIRM_WIDTH = 4,
	TYPICAL_FIRM_HEIGHT = 4
};

// Undefine the worst macro's in existance (courtesy of WinDef.h).
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


// Private functions declared here.
static int RingLength(int R0Width, int R0Height, int R);
static void RingIndexToXY(int R0x, int R0y, int R0Width, int R0Height, int R, int Index, int /*out*/ *x, int /*out*/ *y);





VacateArea::VacateArea()
	: _Width(0), _Height(0), _Area(NULL)
{
	AllocateArea(TYPICAL_FIRM_WIDTH + SCAN_AREA_INCREASE, TYPICAL_FIRM_HEIGHT + SCAN_AREA_INCREASE);
}

VacateArea::~VacateArea()
{
	delete _Area;
}




// Vacate all units of nation_recno that are idle inside the construction area, but do not touch builder_recno.
void VacateArea::VacateIdleOfNation(int buildxLoc, int buildyLoc, int buildWidth, int buildHeight, int nation_recno, int builder_recno)
{
	if (!nation_recno || !builder_recno ||
		buildxLoc < 0 || buildyLoc < 0 || buildWidth < 1 || buildHeight < 1 ||
		buildxLoc + buildWidth >= MAX_WORLD_X_LOC || buildyLoc + buildHeight >= MAX_WORLD_Y_LOC )
	{
		ERR("VacateArea::VacateIdleOfNation was called with bad arguments.\n");
		return;
	}

	// Current algorithm only works properly for SCAN_AREA_INCREASE == 2, but can 'function' without crashing for >= 2.
	assert( SCAN_AREA_INCREASE >= 2 );

	// Select area to scan. Work with a virtual area that can be outside of the map bounds, where any point outside
	// of the accessible area is set to be blocked.

	const int xLoc = buildxLoc - SCAN_AREA_INCREASE; // Upper-left corner of scan area (can be outside of the map).
	const int yLoc = buildyLoc - SCAN_AREA_INCREASE;
	//const int accessibleXLoc = std::max(buildxLoc - SCAN_AREA_INCREASE, 0); // Upper-left corner of accessible scan area
	//const int accessibleYLoc = std::max(buildyLoc - SCAN_AREA_INCREASE, 0);
	//const int accessibleWidth = std::min(buildxLoc + buildWidth + SCAN_AREA_INCREASE, MAX_WORLD_X_LOC) - buildxLoc; // Accessible scan area size
	//const int accessibleHeight = std::min(buildyLoc + buildHeight + 2 * SCAN_AREA_INCREASE, MAX_WORLD_Y_LOC) - buildyLoc;
	const int width = buildWidth + 2 * SCAN_AREA_INCREASE; // Size of scan area (can be in part outside of the map)
	const int height = buildHeight + 2 * SCAN_AREA_INCREASE;
	const int ringCount = std::min(buildWidth + 1 + 2 * SCAN_AREA_INCREASE, buildHeight + 1 + 2 * SCAN_AREA_INCREASE) / 2;
	const int R0Width = buildWidth + 2 * SCAN_AREA_INCREASE - 2 * (ringCount - 1);
	const int R0Height = buildHeight + 2 * SCAN_AREA_INCREASE - 2 * (ringCount - 1);
	const int R0x = ringCount - 1, R0y = ringCount - 1;
	const int outlineRing = ringCount - SCAN_AREA_INCREASE; // The first ring outside of the build zone.

	assert( outlineRing > 0 && ringCount > 0 );
	//assert( accessibleWidth > 0 && accessibleHeight > 0 );

	if ( ! AllocateArea(width, height))
		return;

	int mobileType = unit_array[builder_recno]->mobile_type;
	int occupationCount;

	// Create a schematic representation of the area
	occupationCount = CreateAreaSchematic(mobileType, xLoc, yLoc, width, height, nation_recno, builder_recno, buildxLoc, buildyLoc, buildWidth, buildHeight);

	// All the units that we can move have their recno recorded in the schematic. The challenge is to move the units around in such a way that the build site becomes cleared.
	//
	// Assume that at most one spot of the build zone is inaccessible, in which case it is the builder.
	// The build area is a rectangle of size w * h. The scan area is a rectangle of w+2d * h+2d and can be built up out of disjoint rings, starting from the centre and moving outwards.
	// There are N = min(1+w+2d,1+h+2d)/2 rings. The first ring, R(0), is 2 * 2, 1 * S or S * 1 in size and is at the centre of the area.
	// Successive rings R(i+1) are then the boundary of the smallest rectangle fully containing R(i) in its interior. For i>=1, the length will grow by 8.
	//
	// The algorithm we implement to vacate the build area is:
	//
	// Stage 1: Pushing
	//   Start with the inner ring R(0).
	//   Push[1] all units in the current ring to a spot in the next ring. If empty, mark ring as empty.
	//   Continue moving up in rings until a ring outside the build zone is reached; don't mark rings as empty if a previous ring was non-empty.
	//   Repeat the process with the first non-empty ring, until nothing moved, or until the build zone is vacated.
	//   If the build zone is vacated then we're done; exit. Else, go-to stage 2.
	//
	// Stage 2: Outline filling
	//   Count the number of free (thus necessarily accessible) spots in the ring around the build zone.
	//   Move as many of the units into the free spots, picking always the closest unit to a free spot.
	//   If the build zone is vacated then we're done; exit. Else, go-to stage 3.
	//
	// Stage 3: Filling queues
	//   Let z be the number of units still in the build area. Attempt to free up z spots in the ring R* around the build zone, R*=R(min(w,h)), as follows.
	//	 For any unit in R* that can be moved to a spot in ring R+1 with move-distance=1, move that unit. Pick the closest unit in the build zone and move it to the vacated spot.
	//   Do this for every unit in R or until z spots have been cleared.
	//   If the build zone is vacated then we're done; exit. Else, go-to stage 4.
	//
	// Stage 4: Desperation
	//   We don't know what to do anymore. There really isn't any spot inside our scan area.
	//   Order the remaining units to move to a random spot in R*, preferably one that contains a unit we can move, and hope for the best[2].
	//
	// [1] When pushing, only consider a couple of squares around the unit as viable spots.
	// [2] I suggest we hope for that the units will push each other further as part of the movement, but feel free to hope as you choose.
	//
	// Note 1: Stage 3 is actually the same as Stage 1 only then on a need-to-perform basis, because the ring does not need to be vacated.
	// Note 2: Stage 3 can be augmented for SCAN_AREA_INCREASE > 2 by using the same kind of cycles as for pushing, but if we go there then maybe a different algorithm is appropriate.
	// 
	
	// If there are no movable units inside the build zone then there is nothing to be done.
	if (occupationCount == 0) return;

	// Perform the various move-stages (schematically), as-needed.
	if (occupationCount) occupationCount = PushRings(width, height, R0x, R0y, R0Width, R0Height, outlineRing);
	if (occupationCount) occupationCount = FillOutline(occupationCount, width, height, R0x, R0y, R0Width, R0Height, outlineRing);
	if (occupationCount) occupationCount = PushQueues(occupationCount, width, height, R0x, R0y, R0Width, R0Height, outlineRing);

	// Executed computed moves. Also 'desperately' moves units still stuck inside the build-zone to a random location on the outline ring.
	MoveAccordingToSchematic(width, height, xLoc, yLoc, buildxLoc, buildyLoc, buildWidth, buildHeight, R0x, R0y, R0Width, R0Height, outlineRing);
	
	// TODO: Also account for sprite size and perhaps move magnitude? Update: Sprite size is always 1 (see SpriteRes::loc_width and loc_height). Move magnitude is 1 for all land units and 2 for all navy units.
	// TODO: Marine units as well, for harbour.
}


// Create a schematic of the given area for the given mobileType into _Area. Returns the number of occupants inside the build area (exlcuding the builder); the build area is given in World-coordinates.
int VacateArea::CreateAreaSchematic(int mobileType, int xLoc, int yLoc, int width, int height, int nation_recno, int builder_recno, int buildX, int buildY, int buildWidth, int buildHeight)
{
	static bool __warned_negative_recno = false; // For outputting a diagnostic message only once per program execution.

	int occupancy = 0;
	for (int y = yLoc, j = 0; y < yLoc + height; ++y, ++j)
	{
		for (int x = xLoc, i = 0; x < xLoc + width; ++x, ++i)
		{
			Location *loc = NULL;
			
			// If location is inside the map fetch it.
			if (x >= 0 && x < MAX_WORLD_X_LOC && y >= 0 && y < MAX_WORLD_X_LOC)
				loc = world.get_loc(x,y);
			
			if (!loc || !loc->is_accessible(mobileType)) 
			{
				_Area[j * width + i] = AREA_BLOCKED;
			}
			else if (loc->has_unit(mobileType))
			{
				Unit *unitPtr = unit_array[ loc->unit_recno(mobileType) ];
				assert(unitPtr != NULL);

				// Determine if this unit can be considered an obstacle. If we can order it to move, or if it's moving, then it's not an obstacle.
				
				bool obstacle = true;
				if (unitPtr->cur_action == SPRITE_MOVE && (unitPtr->cur_x_loc() != unitPtr->go_x_loc() || unitPtr->cur_y_loc() != unitPtr->go_y_loc()))
					obstacle = false;
				else if (unitPtr->nation_recno == nation_recno && unitPtr->cur_action == SPRITE_IDLE && unitPtr->action_mode == ACTION_STOP && unitPtr->action_mode2 == ACTION_STOP && (!unitPtr->ai_unit || !unitPtr->ai_action_id))
					obstacle = false;

				// Add results to the schematics

				if (unitPtr->sprite_recno == builder_recno)
				{
					_Area[j * width + i] = AREA_BLOCKED;
				}
				else if (obstacle)
				{
					_Area[j * width + i] = AREA_BLOCKED;
				}
				else
				{
					int unitRecno = unitPtr->sprite_recno;
					if (unitRecno <= 0)
					{
						if (!__warned_negative_recno) ERR("Found negative sprite_recno on unit; can not handle this situation properly. Consider upgrading all shorts to ints for unit recno's.\n");
						__warned_negative_recno = true;
						_Area[j * width + i] = AREA_BLOCKED;			
					}
					else
					{
						_Area[j * width + i] = (loc_flag_t)unitRecno;
						// Count number of (movable) units inside build zone.
						if (x >= buildX && x < buildX + buildWidth && y >= buildY && y < buildY + buildHeight)
							++occupancy;
					}
				}
			}
			else
			{
				_Area[j * width + i] = 0;
			}
		} // for (x ..)
	} // for (y ..)

	return occupancy;
}



// Pushing method: push units from one ring outwards into the next ring. Returns the number of units still in the build zone after pushing.
int VacateArea::PushRings(int width, int height, int R0x, int R0y, int R0Width, int R0Height, int outlineRing)
{
	// Perform cycles of pushing: start from innermost non-empty ring and move to the outline-ring, pushing all units outwards.
	// The cycles stop when all inner rings are empty or a cycle was completed without any units being pushed.

	// Maximum distance a unit is moved this stage.
	enum {MAX_MOVE = 3};

	int occupationCount; // Number of units still in the build zone.
	int emptyRing = -1;
	bool pushed; // Keep track of whether a unit was pushed this cycle.
	do // multiple cycles of pushing
	{
		// Perform a cycle: push from the inner-most non-empty ring to the outline-ring.
		occupationCount = 0;
		pushed = false;
		for (int R = emptyRing + 1; R < outlineRing; ++R)
		{
			const int ringLength = RingLength(R0Width, R0Height, R);
			const int startIndex = misc.random(ringLength);
			bool empty = true;
			// Move all units in the current ring.
			for (int i = 0; i < ringLength; ++i)
			{
				int ringIndex = (startIndex + i) % ringLength;
				int x, y;
				RingIndexToXY(R0x, R0y, R0Width, R0Height, R, ringIndex, &x, &y);
				int flag = _Area[y * width + x];
				if (flag != AREA_BLOCKED && flag != 0)
				{
					// There's a unit on the current spot that needs to be pushed out.
					int unitRecno = flag;
					int nextRingIndex = FindPushedSpot(MAX_MOVE, width, height, R0x, R0y, R0Width, R0Height, R, ringIndex);
					if (nextRingIndex != -1)
					{
						// Move the unit.
						int nextX, nextY;
						RingIndexToXY(R0x, R0y, R0Width, R0Height, R+1, nextRingIndex, &nextX, &nextY);
						assert( _Area[nextY * width + nextX] == 0 );
						_Area[nextY * width + nextX] = unitRecno;
						_Area[y * width + x] = 0;
						pushed = true;
					}
					else
					{
						// There's a unit in the ring that could not be moved.
						empty = false;
						++occupationCount;
					}
				}	
			} // for(i .. )

			if (empty && emptyRing == R-1)
				emptyRing = R;
		} // for(R ..)
	}
	while( emptyRing+1 < outlineRing && pushed );

	assert( (occupationCount == 0) == (emptyRing+1 >= outlineRing) );

	return occupationCount;
}



// Filling method: fill-up outline of build zone. Returns the number of units still in the build zone after pushing.
int VacateArea::FillOutline(int occupationCount, int width, int height, int R0x, int R0y, int R0Width, int R0Height, int outlineRing)
{
	// Note: The assertion is that the only potential blocked point in the build zone is the builder, therefore every spot in the outline area is accessible via an (almost) direct path.

	// Rectangle for the build zone.
	const int buildX = R0x - (outlineRing-1), buildY = R0y - (outlineRing-1);
	const int buildWidth = R0Width + 2 * (outlineRing-1), buildHeight = R0Height + 2 * (outlineRing-1);

	// Fill any free spot in the outline ring with units in the build zone. Always select the unit closest to the free spot.
	const int outlineRingLength = RingLength(R0Width, R0Height, outlineRing);
	const int startIndex = misc.random(outlineRingLength);
	for (int i = 0; i < outlineRingLength && occupationCount > 0; ++i)
	{
		int index = (startIndex + i) % outlineRingLength;
		int x, y;
		RingIndexToXY(R0x, R0y, R0Width, R0Height, outlineRing, index, &x, &y);
		if (_Area[y * width + x] == 0)
		{
			// Found an empty spot; find unit nearest to this spot and move it here.
			int unitX, unitY;
			int unitRecno = FindNearestUnit(x, y, buildX, buildY, buildWidth, buildHeight, width, height, &unitX, &unitY);
			assert(unitRecno != 0 && unitRecno != AREA_BLOCKED);
			if (unitRecno)
			{
				assert( unitX >= 0 && unitX < width && unitY >= 0 && unitY < height );
				assert( _Area[unitY * width + unitX] == unitRecno );
				_Area[y * width + x] = unitRecno;
				_Area[unitY * width + unitX] = 0;
				--occupationCount;
			}
		}
	}

	return occupationCount;
}



// Queueing method: Selectively move as many units as needed from the outline ring to one beyond to make room for the remaining units. (Forms 'queues' around the build spot)
int VacateArea::PushQueues(int occupationCount, int width, int height, int R0x, int R0y, int R0Width, int R0Height, int outlineRing)
{
	// Rectangle for the build zone.
	const int buildX = R0x - (outlineRing-1), buildY = R0y - (outlineRing-1);
	const int buildWidth = R0Width + 2 * (outlineRing-1), buildHeight = R0Height + 2 * (outlineRing-1);

	// Vacate spots in the outline ring by moving units to the next ring and place the remaining units from the build zone. Always select the unit closest to the vacated spot.
	const int outlineRingLength = RingLength(R0Width, R0Height, outlineRing);
	const int startIndex = misc.random(outlineRingLength);
	for (int i = 0; i < outlineRingLength && occupationCount > 0; ++i)
	{
		int index = (startIndex + i) % outlineRingLength;
		int x, y;
		RingIndexToXY(R0x, R0y, R0Width, R0Height, outlineRing, index, &x, &y);
		int flag = _Area[y * width + x];
		if (flag != 0 && flag != AREA_BLOCKED)
		{
			// Found a potentially movable unit; see if it can be moved to the next ring. Then find a unit in the build zone nearest to this spot and move it here.
			int nextRingIndex = FindPushedSpot(1, width, height, R0x, R0y, R0Width, R0Height, outlineRing, index); // Push with max. distance 1.
			if (nextRingIndex != -1)
			{
				// Move unit to the next ring.
				int nextX, nextY;
				RingIndexToXY(R0x, R0y, R0Width, R0Height, outlineRing+1, nextRingIndex, &nextX, &nextY);
				assert( _Area[nextY * width + nextX] == 0 );
				_Area[nextY * width + nextX] = _Area[y * width + x];
				_Area[y * width + x] = 0;
				// Find nearest unit to move into the vacated spot.
				int unitX, unitY;
				int unitRecno = FindNearestUnit(x, y, buildX, buildY, buildWidth, buildHeight, width, height, &unitX, &unitY);
				assert(unitRecno != 0 && unitRecno != AREA_BLOCKED);
				if (unitRecno)
				{
					assert( unitX >= 0 && unitX < width && unitY >= 0 && unitY < height );
					assert( _Area[unitY * width + unitX] == unitRecno );
					_Area[y * width + x] = unitRecno;
					_Area[unitY * width + unitX] = 0;
					--occupationCount;
					// Redo current spot, as there might be more positions to move to.
					--i;
				}
			} //nextRingIndex
		} // flag
	} //for(i ..

	return occupationCount;
}



// Move units according to the schematic in _Area.
void VacateArea::MoveAccordingToSchematic(int width, int height, int xLoc, int yLoc, int buildX, int buildY, int buildWith, int buildHeight, int R0x, int R0y, int R0Width, int R0Height, int outlineRing)
{
	const int outlineRingLength = RingLength(R0Width, R0Height, outlineRing);

	int desperationMoveIndex = misc.random(outlineRingLength);

	for (int y = yLoc, j = 0; y < yLoc + height; ++y, ++j)
	{
		for (int x = xLoc, i = 0; x < xLoc + width; ++x, ++i)
		{
			int flag = _Area[j * width + i];
			if (flag != 0 && flag != AREA_BLOCKED)
			{
				int unitRecno = flag;
				Unit *unit = unit_array[unitRecno];
				assert(unit); if (!unit) continue;
				// Order the unit to move to the area indicated by the schematic.
				// As a last act of desperation, order any units still inside the build zone to a random spot on the outline ring.
				int moveX, moveY;
				if (x >= buildX && x < buildX + buildWith && y >= buildY && y < buildY + buildHeight)
				{
					// Unit inside build zone. Randomly move to the outline ring.
					if (desperationMoveIndex != -1) desperationMoveIndex = FindNextNonblockedInRing(width, height, R0x, R0y, R0Width, R0Height, outlineRing, desperationMoveIndex);
					int moveIndex = (desperationMoveIndex != -1 ? desperationMoveIndex : misc.random(outlineRingLength));
					// Set random move.
					RingIndexToXY(R0x, R0y, R0Width, R0Height, outlineRing, moveIndex, &moveX, &moveY);
				}
				else
				{
					moveX = x;
					moveY = y;
				}

				if (unit->next_x_loc() != moveX || unit->next_y_loc() != moveY)
					unit->move_to(moveX, moveY);
			}
		} // for (y ..)
	} // for (x ..)
}



// Returns the index into R+1 that is the best spot for the unit at Index in R to move to. Returns -1 if it could not find an available spot.
int VacateArea::FindPushedSpot(int maxSearchDistance, int width, int height, int R0x, int R0y, int R0Width, int R0Height, int R, int Index)
{
	// Find an empty spot in the next ring.
	//
	// Formal logic: Let n be the number of points in the outer ring with distance 1 to the current point. Perform a search starting at
	//               the centre-point of all such points with max. search distance d = D + Ceiling(n/2).
	//
	// For efficiency, this logic is split by case, as follows:
	//
	// There are two cases, one for a proper rectangle and one for the non-proper rectangles (1*S, etc.).
	// If it's a proper rectangle, select the point in the outer (next) ring matching the current point and perform a search inside the outer ring for an empty spot starting at the picked point.
	// Corners are matched to corners and the rest are matched with the point directly one up, right, down or left depending on the side the point is at.
	// The second case (only if R=0) has three sub-cases: 1) for 1*1, any point in the outer ring should be checked;
	// 2) for strips (1*S or S*1) the corner points can be checked using the proper-rectangle logic when the point is matched to the point between the two corners of the outer ring and search distance is increased by 2,
	// and 3) the non-corner points can choose between two sides (up/down or left/right), of which one will be picked at random, checked, and if a spot isn't found then the other side is checked.

	const int nextRingLength = RingLength(R0Width, R0Height, R+1);

	if (R == 0 && (R0Width == 1 || R0Height == 1))
	{
		// Not a proper rectangular ring. Seperate three subcases.
		if (R0Width == 1 && R0Height == 1)
		{
			// The centre point. Choose a random direction.
			int startRingIndex = misc.random(nextRingLength);
			return SearchFreeSpotAtIndex((nextRingLength-1) / 2 + ((nextRingLength-1) % 2 == 1), width, height, R0x, R0y, R0Width, R0Height, R+1, startRingIndex);		
		}
		else if (Index == 0 || Index == RingLength(R0Width, R0Height, R)-1)
		{
			// Corner point of strip. Select point between the two corners of the next ring and use standard search-logic with an increased search distance.
			int ringIndex = (R0Width > 1 ? (Index == 0 ? nextRingLength-1 : R0Width + 3) :
				(Index == 0 ? 1 : R0Height + 5));
			return SearchFreeSpotAtIndex(maxSearchDistance + 2, width, height, R0x, R0y, R0Width, R0Height, R+1, ringIndex);
		}
		else
		{
			// Non-corner point of a horizontal or vertical strip.
			int firstDirection = misc.random(2);
			int ringIndex = (R0Width > 1 ? (firstDirection == 0 ? 1 + Index : 1 + R0Width + 3 + (R0Width - 1 - Index)) :
				(firstDirection == 0 ? nextRingLength-1 - Index : 3 + Index));
			int nextIndex = SearchFreeSpotAtIndex(maxSearchDistance, width, height, R0x, R0y, R0Width, R0Height, R+1, ringIndex);
			if (nextIndex >= 0)
				return nextIndex;
			// Did not find an empty spot. Redo for the other direction (exact same code, run with firstDirection inverted).
			firstDirection = !firstDirection;
			ringIndex = (R0Width > 1 ? (firstDirection == 0 ? 1 + Index : 1 + R0Width + 3 + (R0Width - 1 - Index)) :
				(firstDirection == 0 ? nextRingLength-1 - Index : 3 + Index));
			return SearchFreeSpotAtIndex(maxSearchDistance, width, height, R0x, R0y, R0Width, R0Height, R+1, ringIndex);
		}
	}
	else
	{
		// Proper rectangular ring.
		const int UpperLeft = 0, UpperRight = R0Width + 2 * R - 1;
		const int LowerRight = R0Width + 2 * R + R0Height - 1 + 2 * R - 1;
		const int LowerLeft = RingLength(R0Width, R0Height, R) - (R0Height + 2 * R - 1);

		// Determine the point in the next ring that corresponds to the current point.
		bool isCorner;
		int indexShift;
		if (Index <= UpperLeft)
			isCorner = (Index == UpperLeft), indexShift = -1; // (this case only occurs for upper-left corner).
		else if (Index <= UpperRight)
			isCorner = (Index == UpperRight), indexShift = 1;
		else if (Index <= LowerRight)
			isCorner = (Index == LowerRight), indexShift = 3;
		else if (Index <= LowerLeft)
			isCorner = (Index == LowerLeft), indexShift = 5;
		else
			isCorner = false, indexShift = 7;

		int ringIndex = Index + indexShift + isCorner;
		return SearchFreeSpotAtIndex(maxSearchDistance + isCorner, width, height, R0x, R0y, R0Width, R0Height, R+1, ringIndex);
	}
}



// Searches for a free spot in ring R, starting at Index and moving out max. maxSearchDistance to the left and right. Returns the index of the spot, or -1 if no free spot found.
int VacateArea::SearchFreeSpotAtIndex(int maxSearchDistance, int width, int height, int R0x, int R0y, int R0Width, int R0Height, int R, int Index)
{
	const int ringLength = RingLength(R0Width, R0Height, R);
	// Start finding empty spot in the ring from the picked direction.
	int direction = 1;
	for (int i = 0; i < 2 * maxSearchDistance; ++i, direction = -direction)
	{
		const int signedIndex = direction * (i/2 + i%2);
		const int ringIndex = (ringLength + Index + signedIndex) % ringLength; // (ringLength + .. ensures the positive modulus is always taken)
		int x, y;
		RingIndexToXY(R0x, R0y, R0Width, R0Height, R, ringIndex, &x, &y);
		if (_Area[y * width + x] == 0)
			return ringIndex;
	}
	return -1;
}



// Find the nearest unoccupied spot to (locX,locY) inside the area given by the search rectangle (all coordinates are for _Area).
int VacateArea::FindNearestUnit(int locX, int locY, int searchX, int searchY, int searchWidth, int searchHeight, int width, int height, int *unitX, int *unitY)
{
	assert( ! (locX >= searchX && locX < searchX + searchWidth && locY >= searchY && locY < searchY + searchWidth) ); // (locX, locY) should not lie within search area.

	// Assuming the area is not too large, walking through it linearly gives high (and maybe best) performance.
	int bestUnit = 0;
	int bestDistance = INT_MAX;
	int bestDistance2 = INT_MAX; // Secondary distance measure, to favour, on equal distance, units in a straight line over angled.
	for (int y = searchY; y < searchY + searchHeight; ++y)
	{
		for (int x = searchX; x < searchX + searchWidth; ++x)
		{
			int flag = _Area[y * width + x];
			int dist = std::max(std::abs(x - locX), std::abs(y - locY)); // (metric on the map is that of a rectangular grid)
			int dist2 = std::min(std::abs(x - locX), std::abs(y - locY)); // (secondary metric is that of a rectangular lattice: no diagonal movement allowed)
			if (flag != 0 && flag != AREA_BLOCKED && (dist < bestDistance || (dist == bestDistance && dist2 < bestDistance2)))
			{
				bestUnit = flag;
				bestDistance = dist;
				bestDistance2 = dist2;
				*unitX = x; *unitY = y;
				// if (bestDistance == 1) return bestUnit; // Short-circuit checks if the best possible distance is achieved. // Not going to happen in our use-case, because of PushRings. Might consider setting it to 2.
			}
		}
	}

	return bestUnit;
}



// Find the next non-blocked location on the given ring, starting search at prevSearchIndex+1.
int VacateArea::FindNextNonblockedInRing(int width, int height, int R0x, int R0y, int R0Width, int R0Height, int R, int prevSearchIndex)
{
	const int ringLength = RingLength(R0Width, R0Height, R);
	for (int i = 0; i < ringLength; ++i)
	{
		int index = (prevSearchIndex + 1 + i) % ringLength;
		int x, y;
		RingIndexToXY(R0x, R0y, R0Width, R0Height, R, index, &x, &y);
		int flag = _Area[y * width + x];
		if (flag != 0 && flag != AREA_BLOCKED)
		{
			return index;
		}
	}	
	return -1;
}



// Returns the length of ring R.
static inline int RingLength(int R0Width, int R0Height, int R)
{
	if (R > 0 || (R0Width > 1 && R0Height > 1)) // For R > 0 the below formula for a rectangles always applies.
	{
		return 2 * (R0Width + 2 * R) + 2 * (R0Height - 2 + 2 * R);
	}
	else // Starting situation 1 * S or S * 1.
	{
		assert(R0Width == 1 || R0Height == 1);
		return MAX(R0Width, R0Height);
	}
}

// Walks through ring R.
static void RingIndexToXY(int R0x, int R0y, int R0Width, int R0Height, int R, int Index, int /*out*/ *x, int /*out*/ *y)
{
	assert( Index >= 0 && Index < RingLength(R0Width, R0Height, R) );

	// Walk around in a clockwise circle, starting top-left.
	// This code works also for the non-rectangular R=0 starting rings, then it's 1*S or S*1 and will stop after the first (for S*1) or the third (for 1*S) case, because Index can not exceed ring length (S).
	int index = Index;
	if (index < R0Width + 2 * R) // Top
	{
		*x = R0x - R + index, *y = R0y - R;
		return;
	}
	index -= R0Width + 2 * R;
	if (index < R0Height - 2 + 2 * R) // Right-side minus top and bottom
	{
		*x = R0x + R0Width - 1 + R, *y = R0y - R + 1 + index;
		return;
	}
	index -= R0Height - 2 + 2 * R;
	if (index < R0Width + 2 * R) // Bottom
	{
		*x = R0x + R0Width - 1 + R - index, *y = R0y + R0Height - 1 + R;
		return;
	}
	index -= R0Width + 2 * R;
	if (index < R0Height - 2 + 2 * R) // Left
	{
		*x = R0x - R, *y = R0y + R0Height - 1 - 1 + R - index;
		return;
	}

	// This is bad, but don't crash the game because of it. Default to top-left.
	ERR("RingIndexToXY called with Index outside of ring length.\n");
	*x = R0x - R, *y = R0y - R;
}

// Allocate an area of the given size, setting width, height and area, and returns true if successful.
bool VacateArea::AllocateArea(int width, int height)
{
	// If we can fit the new region inside the currently allocated one then we're done.
	if (_Width * _Height >= width * height)
		return true;

	assert( (_Area == NULL) == (_Width == 0 && _Height == 0) );

	delete _Area;

	_Area = new(std::nothrow) loc_flag_t[width * height];
	if (_Area)
		_Width = width, _Height = height;
	else
		_Width = _Height = 0;

	return _Area != NULL;
}
