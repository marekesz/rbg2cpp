#include"automaton_builder.hpp"
#include"game_move.hpp"
#include"state.hpp"
#include"edge.hpp"
#include"actions_block.hpp"
#include"arithmetic_comparison.hpp"
#include"move_check.hpp"
#include"shift.hpp"
#include"ons.hpp"
#include"offs.hpp"
#include"assignments.hpp"
#include"switch.hpp"
#include"power_move.hpp"
#include"star_move.hpp"
#include"concatenation.hpp"
#include"sum.hpp"

uint automaton_builder::current_index = 0;

void automaton_builder::dispatch(const rbg_parser::arithmetic_comparison& m){
    result = edge_automaton(&m, ++current_index);
}

void automaton_builder::dispatch(const rbg_parser::move_check& m){
    result = edge_automaton(&m, ++current_index);
}

void automaton_builder::dispatch(const rbg_parser::shift& m){
    result = edge_automaton(&m, ++current_index);
}

void automaton_builder::dispatch(const rbg_parser::ons& m){
    result = edge_automaton(&m, ++current_index);
}

void automaton_builder::dispatch(const rbg_parser::off& m){
    result = edge_automaton(&m, ++current_index);
}

void automaton_builder::dispatch(const rbg_parser::assignment& m){
    result = edge_automaton(&m, ++current_index);
}

void automaton_builder::dispatch(const rbg_parser::player_switch& m){
    result = edge_automaton(&m, ++current_index);
}

void automaton_builder::dispatch(const rbg_parser::keeper_switch& m){
    result = edge_automaton(&m, ++current_index);
}

void automaton_builder::dispatch(const rbg_parser::actions_block& m){
    std::vector<const rbg_parser::game_move*> edge_labels;
    for(const auto& el: m.get_content()){
        ++current_index;
        edge_labels.push_back(el.get());
    }
    result = edge_automaton(edge_labels, current_index);
}

void automaton_builder::dispatch(const rbg_parser::power_move& m){
    std::vector<automaton> elements;
    for(uint i=0;i<m.get_number_of_repetitions();++i){
        automaton_builder element_builder;
        m.get_content()->accept(element_builder);
        elements.push_back(element_builder.get_final_result());
    }
    result = concatenation_of_automatons(std::move(elements));
}

void automaton_builder::dispatch(const rbg_parser::star_move& m){
    automaton_builder element_builder;
    m.get_content()->accept(element_builder);
    result = element_builder.get_final_result();
    result.starify_automaton();
}

void automaton_builder::dispatch(const rbg_parser::sum& m){
    std::vector<automaton> elements;
    for(const auto& el: m.get_content()){
        automaton_builder element_builder;
        el->accept(element_builder);
        elements.push_back(element_builder.get_final_result());
    }
    result = sum_of_automatons(std::move(elements));
}

void automaton_builder::dispatch(const rbg_parser::concatenation& m){
    std::vector<automaton> elements;
    for(const auto& el: m.get_content()){
        automaton_builder element_builder;
        el->accept(element_builder);
        elements.push_back(element_builder.get_final_result());
    }
    result = concatenation_of_automatons(std::move(elements));
}

automaton automaton_builder::get_final_result(void){
    return std::move(result);
}