
#ifndef __paiv__astar_hpp
#define __paiv__astar_hpp

namespace paiv {

template<typename State>
static bool
compare_cost(State a, State b)
{
    return (a.backward_cost + a.forward_cost)
        > (b.backward_cost + b.forward_cost);
}

template<class State, class Function>
static State
astar(const State& currentState, Function expand)
{
    typedef priority_queue <State, vector<State>, bool(*)(State,State)>
    fringe_t;

    unordered_set<State> closed_set;
    fringe_t fringe(compare_cost);

    fringe.push(currentState);

    while (!fringe.empty())
    {
        State nextState = fringe.top();
        fringe.pop();

        // clog << nextState << endl;

        if (nextState.is_terminal)
            return nextState;

        if (closed_set.find(nextState) != closed_set.end())
            continue;

        closed_set.insert(nextState);

        auto children = expand(nextState);
        for (auto& state : children)
            fringe.push(state);
    }

    return {};
}

}

#endif
