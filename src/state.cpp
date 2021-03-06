#include"state.hpp"

#include<cassert>
#include"cpp_container.hpp"

uint state::next_free_id = 0;

state::state(void):
state_id(next_free_id++),
next_states(),
outgoing_edges_needed(false),
doubly_reachable(false){
}

state::state(const state& rhs):
state_id(next_free_id++),
next_states(rhs.next_states),
outgoing_edges_needed(rhs.outgoing_edges_needed),
doubly_reachable(rhs.doubly_reachable){
}

state& state::operator=(const state& rhs){
    if(this == &rhs)
        return *this;
    state_id = next_free_id++;
    next_states = rhs.next_states;
    outgoing_edges_needed = rhs.outgoing_edges_needed;
    doubly_reachable = rhs.doubly_reachable;
    return *this;
}

uint state::get_id(void)const{
    return state_id;
}

void state::inform_about_being_appended(uint shift_value){
    for(auto& el: next_states)
        el.shift(shift_value);
}

void state::inform_about_state_deletion(uint deleted_index){
    for(auto& el: next_states)
        el.inform_abut_state_deletion(deleted_index);
}

void state::absorb(state&& rhs){
    assert(next_states.empty());
    if(not rhs.next_states.empty()){
        next_states = std::move(rhs.next_states);
        outgoing_edges_needed |= rhs.outgoing_edges_needed;
        doubly_reachable |= rhs.doubly_reachable;
        rhs.next_states.clear();
    }
}

void state::connect_with_state(uint index_in_local_register, const std::vector<label>& label_list, uint index_after_traversing){
    edge result_edge(index_in_local_register);
    for(const auto el: label_list)
        switch(el.k){
            case action:
                result_edge.add_another_action(el.a);
                break;
            case positive_pattern:
                result_edge.add_another_pattern_check(true, el.automaton_index);
                break;
            case negative_pattern:
                result_edge.add_another_pattern_check(false, el.automaton_index);
                break;
        }
    result_edge.set_index(index_after_traversing);
    next_states.push_back(std::move(result_edge));
}

void state::print_transition_functions(
    uint from_state,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::vector<state>& local_register)const{
    if(next_states.size()>1 or outgoing_edges_needed)
        for(const auto& el: next_states)
            el.print_transition_function(from_state, output, pieces_to_id, edges_to_id, variables_to_id, decl, local_register);
}

void state::print_transition_functions_inside_pattern(
    uint from_state,
    uint pattern_index,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::vector<state>& local_register)const{
    if(next_states.size()>1 or outgoing_edges_needed)
        for(const auto& el: next_states)
            el.print_transition_function_inside_pattern(from_state, pattern_index, output, pieces_to_id, edges_to_id, variables_to_id, decl, local_register);
}

void state::print_outgoing_transitions(uint from_state, cpp_container& output, const std::string& functions_prefix)const{
    std::string resulting_line = "{";
    if(next_states.size()>1 or outgoing_edges_needed)
        for(uint i=0;i<next_states.size();++i){
            resulting_line += "&next_states_iterator::"+functions_prefix+"_"+std::to_string(from_state)+"_"+std::to_string(next_states[i].get_endpoint());
            if(i+1<next_states.size())
                resulting_line += ',';
        }
    resulting_line += "},";
    output.add_source_line(resulting_line);
}

void state::notify_endpoints_about_being_reachable(std::vector<uint>& reachability)const{
    for(const auto& el: next_states)
        ++reachability[el.get_endpoint()];
}

void state::mark_as_doubly_reachable(void){
    doubly_reachable = true;
}

bool state::can_be_checked_for_visit(void)const{
    return doubly_reachable;
}

void state::mark_explicitly_as_transition_start(void){
    outgoing_edges_needed = true;
}

const edge& state::get_only_exit(void)const{
    assert(is_no_choicer());
    return next_states[0];
}

bool state::is_dead_end(void)const{
    return next_states.empty();
}

bool state::is_no_choicer(void)const{
    return next_states.size() == 1;
}
