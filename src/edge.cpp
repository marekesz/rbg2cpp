#include"edge.hpp"
#include"game_move.hpp"
#include"cpp_container.hpp"
#include"actions_compiler.hpp"
#include"declarations.hpp"
#include"state.hpp"

edge::edge(uint local_register_endpoint_index):
local_register_endpoint_index(local_register_endpoint_index),
label_list(),
index_after_traversing(0){
}

void edge::add_another_action(const rbg_parser::game_move* a){
    label_list.push_back({action,a,0});
}

void edge::add_another_pattern_check(bool positive, uint automaton_index){
    label_list.push_back({(positive?positive_pattern:negative_pattern),nullptr,automaton_index});
}

void edge::shift(uint shift_value){
    local_register_endpoint_index += shift_value;
}

void edge::inform_abut_state_deletion(uint deleted_index){
    if(local_register_endpoint_index>deleted_index)
        --local_register_endpoint_index;
}

void edge::set_index(uint index){
    index_after_traversing = index;
}

uint edge::get_endpoint(void)const{
    return local_register_endpoint_index;
}

void edge::handle_labels(cpp_container& output, actions_compiler& ac, const std::string& revert_name)const{
    for(const auto& el: label_list)
        switch(el.k){
            case action:
                el.a->accept(ac);
                break;
            case positive_pattern:
                ac.finallize();
                output.add_source_line("evaluate"+std::to_string(el.automaton_index)+"();");
                output.add_source_line("if(not success_to_report"+std::to_string(el.automaton_index)+"){");
                output.add_source_line(revert_name);
                output.add_source_line("return;");
                output.add_source_line("}");
                break;
            case negative_pattern:
                ac.finallize();
                output.add_source_line("evaluate"+std::to_string(el.automaton_index)+"();");
                output.add_source_line("if(success_to_report"+std::to_string(el.automaton_index)+"){");
                output.add_source_line(revert_name);
                output.add_source_line("return;");
                output.add_source_line("}");
                break;
        }
}

void edge::visit_node(cpp_container& output, uint current_state, actions_compiler& ac)const{
    ac.finallize();
    output.add_source_line("if(cache.is_set("+std::to_string(current_state)+", state_to_change.current_cell-1)){");
    output.add_source_line("revert_to_last_choice();");
    output.add_source_line("return;");
    output.add_source_line("}");
    output.add_source_line("cache.set("+std::to_string(current_state)+", state_to_change.current_cell-1);");
}

void edge::visit_node_in_pattern(cpp_container& output, uint current_state, uint pattern_index, actions_compiler& ac)const{
    ac.finallize();
    output.add_source_line("if(cache.pattern_is_set"+std::to_string(pattern_index)+"("+std::to_string(current_state)+", state_to_change.current_cell-1)){");
    output.add_source_line("revert_to_last_choice_because_failure(&next_states_iterator::pattern_decision_points"+std::to_string(pattern_index)+",pattern_transitions"+std::to_string(pattern_index)+",&resettable_bitarray_stack::pattern_revert_to_level"+std::to_string(pattern_index)+");");
    output.add_source_line("return;");
    output.add_source_line("}");
    output.add_source_line("cache.pattern_set"+std::to_string(pattern_index)+"("+std::to_string(current_state)+", state_to_change.current_cell-1);");
}

void edge::print_transition_function(
    uint from_state,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::vector<state>& local_register)const{
    output.add_header_line("void transition_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(void);");
    output.add_source_line("void next_states_iterator::transition_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(void){");
    if(local_register[local_register_endpoint_index].is_dead_end()){
        output.add_source_line("revert_to_last_choice();");
    }
    else{
        std::string revert = "revert_to_last_choice();";
        std::string pusher = "push()";
        actions_compiler ac(output,pieces_to_id,edges_to_id,variables_to_id,decl,revert,pusher,true);
        handle_labels(output,ac,revert);
        uint current_state = local_register_endpoint_index;
        while(not ac.is_ready_to_report() and local_register[current_state].is_no_choicer()){
            if(local_register[current_state].can_be_checked_for_visit())
                visit_node(output, current_state,ac);
            local_register[current_state].get_only_exit().handle_labels(output,ac,revert);
            current_state = local_register[current_state].get_only_exit().local_register_endpoint_index;
        }
        if(local_register[current_state].can_be_checked_for_visit())
            visit_node(output, current_state,ac);
        ac.finallize();
        if(ac.is_ready_to_report()){
            output.add_source_line("ready_to_report = true;");
            output.add_source_line("state_to_change.current_state = "+std::to_string(current_state)+";");
        }
        else{
            output.add_source_line("decision_points.emplace_back(0,"+std::to_string(current_state)+",state_to_change.current_cell,cache.get_level(),board_change_points.size(),variables_change_points.size());");
        }
    }
    output.add_source_line("}");
    output.add_source_line("");
}

void edge::print_transition_function_inside_pattern(
    uint from_state,
    uint pattern_index,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::vector<state>& local_register)const{
    output.add_header_line("void pattern_transition"+std::to_string(pattern_index)+"_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(void);");
    output.add_source_line("void next_states_iterator::pattern_transition"+std::to_string(pattern_index)+"_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(void){");
    std::string revert = "revert_to_last_choice_because_failure(&next_states_iterator::pattern_decision_points"+std::to_string(pattern_index)+",pattern_transitions"+std::to_string(pattern_index)+",&resettable_bitarray_stack::pattern_revert_to_level"+std::to_string(pattern_index)+");";
    std::string pusher = "pattern_push"+std::to_string(pattern_index)+"()";
    actions_compiler ac(output,pieces_to_id,edges_to_id,variables_to_id,decl,revert,pusher,false);
    handle_labels(output,ac,revert);
    uint current_state = local_register_endpoint_index;
    while(local_register[current_state].is_no_choicer()){
        if(local_register[current_state].can_be_checked_for_visit())
            visit_node_in_pattern(output,current_state,pattern_index,ac);
        local_register[current_state].get_only_exit().handle_labels(output,ac,revert);
        current_state = local_register[current_state].get_only_exit().local_register_endpoint_index;
    }
    if(local_register[current_state].can_be_checked_for_visit())
        visit_node_in_pattern(output,current_state,pattern_index,ac);
    ac.finallize();
    assert(not ac.is_ready_to_report());
    if(local_register[current_state].is_dead_end()){
        output.add_source_line("success_to_report"+std::to_string(pattern_index)+" = true;");
        output.add_source_line("revert_to_point_after_pattern(pattern_decision_points"+std::to_string(pattern_index)+"[0], &resettable_bitarray_stack::pattern_revert_to_level"+std::to_string(pattern_index)+");");
        output.add_source_line("pattern_decision_points"+std::to_string(pattern_index)+".clear();");
    }
    else
        output.add_source_line("pattern_decision_points"+std::to_string(pattern_index)+".emplace_back(0,"+std::to_string(current_state)+",state_to_change.current_cell,cache.pattern_get_level"+std::to_string(pattern_index)+"(),board_change_points.size(),variables_change_points.size());");
    output.add_source_line("}");
    output.add_source_line("");
}
