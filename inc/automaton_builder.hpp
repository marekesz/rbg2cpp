#ifndef AUTOMATON_BUILDER
#define AUTOMATON_BUILDER

#include"abstract_dispatcher.hpp"
#include"automaton.hpp"
#include"types.hpp"
#include<cassert>
#include<vector>

namespace rbg_parser{
    class game_move;
}

class automaton_builder : public rbg_parser::abstract_dispatcher{
        automaton result;
        static uint current_index;
        bool should_assign_indices;
        std::vector<automaton>& pattern_automata;
        bool block_building_mode;
        bool block_has_switch;
        std::vector<label> current_block;
    public:
        automaton_builder(const automaton_builder&)=delete;
        automaton_builder(automaton_builder&&)=default;
        automaton_builder& operator=(const automaton_builder&)=delete;
        automaton_builder& operator=(automaton_builder&&)=default;
        ~automaton_builder(void)override=default;
        automaton_builder(std::vector<automaton>& pattern_automata, bool should_assign_indices=true);
        void dispatch(const rbg_parser::sum& m)override;
        void dispatch(const rbg_parser::concatenation& m)override;
        void dispatch(const rbg_parser::star_move& m)override;
        void dispatch(const rbg_parser::shift& m)override;
        void dispatch(const rbg_parser::ons& m)override;
        void dispatch(const rbg_parser::off& m)override;
        void dispatch(const rbg_parser::assignment& m)override;
        void dispatch(const rbg_parser::player_switch& m)override;
        void dispatch(const rbg_parser::keeper_switch& m)override;
        void dispatch(const rbg_parser::move_check& m)override;
        void dispatch(const rbg_parser::actions_block& m)override;
        void dispatch(const rbg_parser::arithmetic_comparison& m)override;
        void dispatch(const rbg_parser::integer_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::variable_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::arithmetic_operation&)override{assert(false);}
        automaton get_final_result(void);
};

#endif
