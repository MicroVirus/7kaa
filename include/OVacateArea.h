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

//Filename    : OVacateArea.h
//Description : Helper class/function to order units to vacate a specific area.
//              Used for constructing buildings and settling towns, when a unit is blocking
//              the build spot Vacate Area can order offending units out of the way.

#ifndef __OVACATEAREA_H
#define __OVACATEAREA_H

#include <climits>

class VacateArea
{
public:
	VacateArea();
	~VacateArea();

	// Vacate all units of nation_recno that are idle inside the construction area, but do not touch builder_recno.
	void VacateIdleOfNation(int x, int y, int width, int height, int nation_recno, int builder_recno);


private:
	// Allocate an area of the given size, setting width, height and area, and returns true if successful.
	bool AllocateArea(int width, int height);
	// Create a schematic of the given area for the given mobileType into _Area. Returns the number of occupants inside the build area (exlcuding the builder); the build area is given in _Area-coordinates (not map coordinates).
	int CreateAreaSchematic(int mobileType, int x, int y, int width, int height, int nation_recno, int builder_recno, int buildX, int buildY, int buildWidth, int buildHeight);
	// Pushing method: push units from one ring outwards into the next ring. Returns the number of units still in the build zone after pushing.
	int PushRings(int width, int height, int R0x, int R0y, int R0Width, int R0Height, int surroundRing);
	// Filling method: fill-up outline of build zone. Returns the number of units still in the build zone after pushing.
	int FillOutline(int occupationCount, int width, int height, int R0x, int R0y, int R0Width, int R0Height, int outlineRing);
	// Queueing method: Selectively move as many units as needed from the outline ring to one beyond to make room for the remaining units. (Forms 'queues' around the build spot)
	int PushQueues(int occupationCount, int width, int height, int R0x, int R0y, int R0Width, int R0Height, int outlineRing);
	// Move units according to the schematic in _Area.
	void MoveAccordingToSchematic(int width, int height, int xLoc, int yLoc, int buildX, int buildY, int buildWith, int buildHeight, int R0x, int R0y, int R0Width, int R0Height, int outlineRing);
	// Returns the index into R+1 that is the best spot for the unit at Index in R to move to.
	int FindPushedSpot(int maxSearchDistance, int width, int height, int R0x, int R0y, int R0Width, int R0Height, int R, int Index);
	// Searches for a free spot in ring R, starting at Index and moving out max. maxSearchDistance to the left and right. Returns the index of the spot, or -1 if no free spot found.
	int SearchFreeSpotAtIndex(int maxSearchDistance, int width, int height, int R0x, int R0y, int R0Width, int R0Height, int R, int Index);
	// Find the nearest unoccupied spot to (x,y) inside the area given by the search rectangle (all coordinates are for _Area).
	int FindNearestUnit(int x, int y, int searchX, int searchY, int searchWidth, int searchHeight, int width, int height, int *unitX, int *unitY);
	// Find the next non-blocked location on the given ring, starting search at prevSearchIndex+1.
	int FindNextNonblockedInRing(int width, int height, int R0x, int R0y, int R0Width, int R0Height, int R, int prevSearchIndex);
	
private:
	// Define unsigned type for location flags. Must be capable of holding all possible unit recno's plus a flag in the final bit.
	typedef unsigned int loc_flag_t;

	int          _Width;
	int          _Height;
	loc_flag_t * _Area;    // Array of width*height elements, used to cache the area.

	enum {
		// Last bit of location flag is used to indicate if the area is blocked by something non-movable.
		AREA_BLOCKED = (loc_flag_t)1 << (sizeof(loc_flag_t) * CHAR_BIT - 1),
		// Number of locations to scan beyond the construction area, so the scanned area is (construction size + scan increase)^2. Current algorithm asserts >= 2 and does not benefit from >2.
		SCAN_AREA_INCREASE = 2
	};

private:
	// Class is not capable of being copied, though if needed this can be readily implemented
	VacateArea(const VacateArea &);
	VacateArea& operator=(const VacateArea &);
};

#endif
