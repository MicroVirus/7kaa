/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 1997,1998 Enlight Software Ltd.
* Copyright 2010 Unavowed <unavowed@vexillium.org>
* Copyright 2017 Richard Dijk <microvirus.multiplying@gmail.com>
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

#include <OUNIT.h>
#include <OU_MARI.h>
#include <OU_CARA.h>
#include <OU_CART.h>
#include <OU_MONS.h>
#include <OU_VEHI.h>
#include <OU_GOD.h>
#include <file_io_visitor.h>
#include <visit_sprite.h>

using namespace FileIOVisitor;


template <typename Visitor>
static void visit_unit_members(Visitor *v, Unit *u)
{
	/* Sprite */
	visit_sprite_members(v, u);

	/* Unit */
	visit<int8_t>(v, &u->unit_id);
	visit<int8_t>(v, &u->rank_id);
	visit<int8_t>(v, &u->race_id);
	visit<int8_t>(v, &u->nation_recno);
	visit<int8_t>(v, &u->ai_unit);
	visit<uint16_t>(v, &u->name_id);
	visit<uint32_t>(v, &u->unit_group_id);
	visit<uint32_t>(v, &u->team_id);
	visit<int8_t>(v, &u->selected_flag);
	visit<int8_t>(v, &u->group_select_id);
	visit<int8_t>(v, &u->waiting_term);
	visit<int8_t>(v, &u->blocked_by_member);
	visit<int8_t>(v, &u->swapping);
	visit<int16_t>(v, &u->leader_unit_recno);
	visit<int8_t>(v, &u->action_misc);
	visit<int16_t>(v, &u->action_misc_para);
	visit<int8_t>(v, &u->action_mode);
	visit<int16_t>(v, &u->action_para);
	visit<int16_t>(v, &u->action_x_loc);
	visit<int16_t>(v, &u->action_y_loc);
	visit<int8_t>(v, &u->action_mode2);
	visit<int16_t>(v, &u->action_para2);
	visit<int16_t>(v, &u->action_x_loc2);
	visit<int16_t>(v, &u->action_y_loc2);
	visit_array<int8_t>(v, u->blocked_edge);
	visit<uint8_t>(v, &u->attack_dir);
	visit<int16_t>(v, &u->range_attack_x_loc);
	visit<int16_t>(v, &u->range_attack_y_loc);
	visit<int16_t>(v, &u->move_to_x_loc);
	visit<int16_t>(v, &u->move_to_y_loc);
	visit<int8_t>(v, &u->loyalty);
	visit<int8_t>(v, &u->target_loyalty);
	visit<float>(v, &u->hit_points);
	visit<int16_t>(v, &u->max_hit_points);

	visit<int8_t>(v, &u->skill.combat_level);
	visit<int8_t>(v, &u->skill.skill_id);
	visit<int8_t>(v, &u->skill.skill_level);
	visit<uint8_t>(v, &u->skill.combat_level_minor);
	visit<uint8_t>(v, &u->skill.skill_level_minor);
	visit<uint8_t>(v, &u->skill.skill_potential);

	visit<int8_t>(v, &u->unit_mode);
	visit<int16_t>(v, &u->unit_mode_para);
	visit<int16_t>(v, &u->spy_recno);
	visit<int16_t>(v, &u->nation_contribution);
	visit<int16_t>(v, &u->total_reward);
	visit_pointer(v, &u->attack_info_array);
	visit<int8_t>(v, &u->attack_count);
	visit<int8_t>(v, &u->attack_range);
	visit<int16_t>(v, &u->cur_power);
	visit<int16_t>(v, &u->max_power);
	visit_pointer(v, &u->result_node_array);
	visit<int32_t>(v, &u->result_node_count);
	visit<int16_t>(v, &u->result_node_recno);
	visit<int16_t>(v, &u->result_path_dist);
	visit_pointer(v, &u->way_point_array);
	visit<int16_t>(v, &u->way_point_array_size);
	visit<int16_t>(v, &u->way_point_count);
	visit<uint16_t>(v, &u->ai_action_id);
	visit<int8_t>(v, &u->original_action_mode);
	visit<int16_t>(v, &u->original_action_para);
	visit<int16_t>(v, &u->original_action_x_loc);
	visit<int16_t>(v, &u->original_action_y_loc);
	visit<int16_t>(v, &u->original_target_x_loc);
	visit<int16_t>(v, &u->original_target_y_loc);
	visit<int16_t>(v, &u->ai_original_target_x_loc);
	visit<int16_t>(v, &u->ai_original_target_y_loc);
	visit<int8_t>(v, &u->ai_no_suitable_action);
	visit<int8_t>(v, &u->can_guard_flag);
	visit<int8_t>(v, &u->can_attack_flag);
	visit<int8_t>(v, &u->force_move_flag);
	visit<int16_t>(v, &u->home_camp_firm_recno);
	visit<int8_t>(v, &u->aggressive_mode);
	visit<int8_t>(v, &u->seek_path_fail_count);
	visit<int8_t>(v, &u->ignore_power_nation);
	visit_pointer(v, &u->team_info);
}

template <typename Visitor>
static void visit_result_node_members(Visitor* v, ResultNode* c)
{
	visit<int16_t>(v, &c->node_x);
	visit<int16_t>(v, &c->node_y);
}

template <typename Visitor>
static void visit_team_info_members(Visitor* v, TeamInfo* c)
{
	visit<int8_t>(v, &c->member_count);
	visit_array<int16_t>(v, c->member_unit_array);
	visit<int32_t>(v, &c->ai_last_request_defense_date);
}

template <typename Visitor>
void visit_unit_members_array(Visitor* v, Unit* unit, bool is_reader_visitor)
{
	if( unit->result_node_array )
	{
		if (is_reader_visitor)
			unit->result_node_array = (ResultNode*) mem_add( sizeof(ResultNode) * unit->result_node_count );

		v->with_record_size(unit->result_node_count*sizeof(ResultNode));
		for (int i = 0; i < unit->result_node_count; ++i) {
			visit_result_node_members(v, &unit->result_node_array[i]);
		}
	}

	//### begin alex 15/10 ###//
	if( unit->way_point_array )
	{
		if (is_reader_visitor)
			unit->way_point_array = (ResultNode*) mem_add(sizeof(ResultNode) * unit->way_point_array_size);

		v->with_record_size(unit->way_point_array_size*sizeof(ResultNode));
		for (int i = 0; i < unit->way_point_array_size; ++i) {
			visit_result_node_members(v, &unit->way_point_array[i]);
		}
	}
	//#### end alex 15/10 ####//

	if( unit->team_info )
	{
		if (is_reader_visitor)
			unit->team_info = (TeamInfo*) mem_add( sizeof(TeamInfo) );

		v->with_record_size(23);
		visit_team_info_members(v, &*unit->team_info);
	}
}

template <typename Visitor>
static void visit_trade_stop_members(Visitor *v, TradeStop *ts)
{
	visit<int16_t>(v, &ts->firm_recno);
	visit<int16_t>(v, &ts->firm_loc_x1);
	visit<int16_t>(v, &ts->firm_loc_y1);
	visit<int8_t>(v, &ts->pick_up_type);
	visit_array<int8_t>(v, ts->pick_up_array);
}

template <typename Visitor>
static void visit_ship_stop_members(Visitor *v, ShipStop *c)
{
	visit_trade_stop_members(v, c);
}

template <typename Visitor>
static void visit_attack_info(Visitor *v, AttackInfo *ai)
{
	visit<uint8_t>(v, &ai->combat_level);
	visit<uint8_t>(v, &ai->attack_delay);
	visit<uint8_t>(v, &ai->attack_range);
	visit<uint8_t>(v, &ai->attack_damage);
	visit<uint8_t>(v, &ai->pierce_damage);
	visit<int16_t>(v, &ai->bullet_out_frame);
	visit<int8_t>(v, &ai->bullet_speed);
	visit<int8_t>(v, &ai->bullet_radius);
	visit<int8_t>(v, &ai->bullet_sprite_id);
	visit<int8_t>(v, &ai->dll_bullet_sprite_id);
	visit<int8_t>(v, &ai->eqv_attack_next);
	visit<int16_t>(v, &ai->min_power);
	visit<int16_t>(v, &ai->consume_power);
	visit<int8_t>(v, &ai->fire_radius);
	visit<int16_t>(v, &ai->effect_id);
}

template <typename Visitor>
static void visit_unit_marine_members(Visitor *v, UnitMarine *u)
{
	visit_sprite_members(v, &u->splash);
	visit<int8_t>(v, &u->menu_mode);
	visit<int8_t>(v, &u->extra_move_in_beach);
	visit<int8_t>(v, &u->in_beach);
	visit<int8_t>(v, &u->selected_unit_id);
	visit_array<int16_t>(v, u->unit_recno_array);
	visit<int8_t>(v, &u->unit_count);
	visit<int8_t>(v, &u->journey_status);
	visit<int8_t>(v, &u->dest_stop_id);
	visit<int8_t>(v, &u->stop_defined_num);
	visit<int8_t>(v, &u->wait_count);
	visit<int16_t>(v, &u->stop_x_loc);
	visit<int16_t>(v, &u->stop_y_loc);
	visit<int8_t>(v, &u->auto_mode);
	visit<int16_t>(v, &u->cur_firm_recno);
	visit<int16_t>(v, &u->carry_goods_capacity);
	visit_array(v, u->stop_array, visit_ship_stop_members<Visitor>);
	visit_array<int16_t>(v, u->raw_qty_array);
	visit_array<int16_t>(v, u->product_raw_qty_array);
	visit_attack_info(v, &u->ship_attack_info);
	visit<uint8_t>(v, &u->attack_mode_selected);
	visit<int32_t>(v, &u->last_load_goods_date);
}

template <typename Visitor>
static void visit_caravan_stop_members(Visitor* v, CaravanStop* c)
{
	visit_trade_stop_members(v, c);
	visit<int8_t>(v, &c->firm_id);
}

template <typename Visitor>
static void visit_unit_caravan_members(Visitor* v, UnitCaravan* c)
{
	visit<int16_t>(v, &c->caravan_id);
	visit<int8_t>(v, &c->journey_status);
	visit<int8_t>(v, &c->dest_stop_id);
	visit<int8_t>(v, &c->stop_defined_num);
	visit<int8_t>(v, &c->wait_count);
	visit<int16_t>(v, &c->stop_x_loc);
	visit<int16_t>(v, &c->stop_y_loc);
	visit_array(v, c->stop_array, visit_caravan_stop_members<Visitor>);
	visit<int32_t>(v, &c->last_set_stop_date);
	visit<int32_t>(v, &c->last_load_goods_date);
	visit_array<int16_t>(v, c->raw_qty_array);
	visit_array<int16_t>(v, c->product_raw_qty_array);
}

template <typename Visitor>
static void visit_unit_exp_cart_members(Visitor* v, UnitExpCart* c)
{
	visit<int8_t>(v, &c->triggered);
}

template <typename Visitor>
static void visit_unit_monster_members(Visitor* v, UnitMonster* c)
{
	visit<int8_t>(v, &c->monster_action_mode);
}

template <typename Visitor>
static void visit_unit_vehicle_members(Visitor* v, UnitVehicle* c)
{
	visit<int16_t>(v, &c->solider_hit_points);
	visit<int16_t>(v, &c->vehicle_hit_points);
}

template <typename Visitor>
static void visit_unit_god_members(Visitor* v, UnitGod* c)
{
	visit<int16_t>(v, &c->god_id);
	visit<int16_t>(v, &c->base_firm_recno);
	visit<int8_t>(v, &c->cast_power_type);
	visit<int16_t>(v, &c->cast_origin_x);
	visit<int16_t>(v, &c->cast_origin_y);
	visit<int16_t>(v, &c->cast_target_x);
	visit<int16_t>(v, &c->cast_target_y);
}


// ===============================================================================

void Unit::accept_file_visitor(FileReaderVisitor* v)
{
	visit_unit_members(v, this);
	visit_unit_members_array(v, this, true);
	
	//----------- post-process the data read ----------//

	// attack_info_array = unit_res.attack_info_array+unit_res[unit_id]->first_attack-1;
	sprite_info = sprite_res[sprite_id];
	sprite_info->load_bitmap_res();
}

void Unit::accept_file_visitor(FileWriterVisitor* v)
{
	visit_unit_members(v, this);
	visit_unit_members_array(v, this, false);
}

enum { UNIT_MARINE_DERIVED_RECORD_SIZE = 145 };

void UnitMarine::accept_file_visitor(FileReaderVisitor* v)
{
	Unit::accept_file_visitor(v);
	v->with_record_size(UNIT_MARINE_DERIVED_RECORD_SIZE);
	visit_unit_marine_members(v, this);

	//----------- post-process the data read ----------//

	splash.sprite_info = sprite_res[splash.sprite_id];
	splash.sprite_info->load_bitmap_res();
}

void UnitMarine::accept_file_visitor(FileWriterVisitor* v)
{
	Unit::accept_file_visitor(v);
	v->with_record_size(UNIT_MARINE_DERIVED_RECORD_SIZE);
	visit_unit_marine_members(v, this);
}

enum { UNIT_CARAVAN_DERIVED_RECORD_SIZE = 72 };

void UnitCaravan::accept_file_visitor(FileReaderVisitor* v)
{
	Unit::accept_file_visitor(v);
	v->with_record_size(UNIT_CARAVAN_DERIVED_RECORD_SIZE);
	visit_unit_caravan_members(v, this);
}

void UnitCaravan::accept_file_visitor(FileWriterVisitor* v)
{
	Unit::accept_file_visitor(v);
	v->with_record_size(UNIT_CARAVAN_DERIVED_RECORD_SIZE);
	visit_unit_caravan_members(v, this);
}

enum { UNIT_EXP_CART_DERIVED_RECORD_SIZE = 1 };

void UnitExpCart::accept_file_visitor(FileReaderVisitor* v)
{
	Unit::accept_file_visitor(v);
	v->with_record_size(UNIT_EXP_CART_DERIVED_RECORD_SIZE);
	visit_unit_exp_cart_members(v, this);
}

void UnitExpCart::accept_file_visitor(FileWriterVisitor* v)
{
	Unit::accept_file_visitor(v);
	v->with_record_size(UNIT_EXP_CART_DERIVED_RECORD_SIZE);
	visit_unit_exp_cart_members(v, this);
}

enum { UNIT_MONSTER_DERIVED_RECORD_SIZE = 1 };

void UnitMonster::accept_file_visitor(FileReaderVisitor* v)
{
	Unit::accept_file_visitor(v);
	v->with_record_size(UNIT_MONSTER_DERIVED_RECORD_SIZE);
	visit_unit_monster_members(v, this);
}

void UnitMonster::accept_file_visitor(FileWriterVisitor* v)
{
	Unit::accept_file_visitor(v);
	v->with_record_size(UNIT_MONSTER_DERIVED_RECORD_SIZE);
	visit_unit_monster_members(v, this);
}

enum { UNIT_VEHICLE_DERIVED_RECORD_SIZE = 4 };

void UnitVehicle::accept_file_visitor(FileReaderVisitor* v)
{
	Unit::accept_file_visitor(v);
	v->with_record_size(UNIT_VEHICLE_DERIVED_RECORD_SIZE);
	visit_unit_vehicle_members(v, this);
}

void UnitVehicle::accept_file_visitor(FileWriterVisitor* v)
{
	Unit::accept_file_visitor(v);
	v->with_record_size(UNIT_VEHICLE_DERIVED_RECORD_SIZE);
	visit_unit_vehicle_members(v, this);
}

enum { UNIT_GOD_DERIVED_RECORD_SIZE = 13 };

void UnitGod::accept_file_visitor(FileReaderVisitor* v)
{
	Unit::accept_file_visitor(v);
	v->with_record_size(UNIT_GOD_DERIVED_RECORD_SIZE);
	visit_unit_god_members(v, this);
}

void UnitGod::accept_file_visitor(FileWriterVisitor* v)
{
	Unit::accept_file_visitor(v);
	v->with_record_size(UNIT_GOD_DERIVED_RECORD_SIZE);
	visit_unit_god_members(v, this);
}


enum { UNIT_RECORD_SIZE = 169 };

//-------- Start of function UnitArray::write_file -------------//
//
int UnitArray::write_file(File* filePtr)
{
	int  i;
	Unit *unitPtr;

	filePtr->file_put_short(restart_recno);  // variable in SpriteArray

	filePtr->file_put_short( size()  );  // no. of units in unit_array

	filePtr->file_put_short( selected_recno );
	filePtr->file_put_short( selected_count );
	filePtr->file_put_long ( cur_group_id   );
	filePtr->file_put_long ( cur_team_id    );
	filePtr->file_put_short(idle_blocked_unit_reset_count);
	filePtr->file_put_long (unit_search_tries);
	filePtr->file_put_short(unit_search_tries_flag);

	filePtr->file_put_short(visible_unit_count);
	filePtr->file_put_short(mp_first_frame_to_select_caravan);
	filePtr->file_put_short(mp_first_frame_to_select_ship);
	filePtr->file_put_short(mp_pre_selected_caravan_recno);
	filePtr->file_put_short(mp_pre_selected_ship_recno);

	for( i=1; i<=size() ; i++ )
	{
		unitPtr = (Unit*) get_ptr(i);

		//----- write unitId or 0 if the unit is deleted -----//

		if( !unitPtr )    // the unit is deleted
		{
			filePtr->file_put_short(0);
		}
		else
		{
			//--------- write unit_id -------------//

			filePtr->file_put_short(unitPtr->unit_id);

			//------ write data ------//

			if( !polymorphic_visit_with_record_size<FileWriterVisitor>(filePtr, unitPtr, UNIT_RECORD_SIZE) )
				return 0;
		}
	}

	//------- write empty room array --------//

	{
		FileWriterVisitor v(filePtr);
		visit_empty_room_array(&v);
	}

	return 1;
}
//--------- End of function UnitArray::write_file ---------------//


//-------- Start of function UnitArray::read_file -------------//
//
int UnitArray::read_file(File* filePtr)
{
	Unit*   unitPtr;
	int     i, unitId, emptyRoomCount=0;

	restart_recno    = filePtr->file_get_short();

	int unitCount    = filePtr->file_get_short();  // get no. of units from file

	selected_recno   = filePtr->file_get_short();
	selected_count   = filePtr->file_get_short();
	cur_group_id     = filePtr->file_get_long();
	cur_team_id      = filePtr->file_get_long();
	idle_blocked_unit_reset_count = filePtr->file_get_short();
	unit_search_tries	= filePtr->file_get_long ();
	unit_search_tries_flag = (char) filePtr->file_get_short();

	visible_unit_count					= filePtr->file_get_short();
	mp_first_frame_to_select_caravan = (char) filePtr->file_get_short();
	mp_first_frame_to_select_ship		= (char) filePtr->file_get_short();
	mp_pre_selected_caravan_recno		= filePtr->file_get_short();
	mp_pre_selected_ship_recno			= filePtr->file_get_short();

	for( i=1 ; i<=unitCount ; i++ )
	{
		unitId = filePtr->file_get_short();

		if( unitId==0 )  // the unit has been deleted
		{
			add_blank(1);     // it's a DynArrayB function
			emptyRoomCount++;
		}
		else
		{
			//----- create unit object -----------//

			unitPtr = create_unit( unitId );
			unitPtr->unit_id = unitId;

			//---- read data -----//

			if( !polymorphic_visit_with_record_size<FileReaderVisitor>(filePtr, unitPtr, UNIT_RECORD_SIZE) )
				return 0;

			unitPtr->fix_attack_info();
		}
	}

	//-------- linkout() those record added by add_blank() ----------//
	//-- So they will be marked deleted in DynArrayB and can be -----//
	//-- undeleted and used when a new record is going to be added --//

	for( i=size() ; i>0 ; i-- )
	{
		DynArrayB::go(i);             // since UnitArray has its own go() which will call GroupArray::go()

		if( get_ptr() == NULL )       // add_blank() record
			linkout();
	}

	//------- read empty room array --------//

	{
		FileReaderVisitor v(filePtr);
		visit_empty_room_array(&v);
	}

	//------- verify the empty_room_array loading -----//

#ifdef DEBUG
	err_when( empty_room_count != emptyRoomCount );

	for( i=0 ; i<empty_room_count ; i++ )
	{
		if( !is_deleted( empty_room_array[i].recno ) )
			err_here();
	}
#endif

	return 1;
}
//--------- End of function UnitArray::read_file ---------------//
