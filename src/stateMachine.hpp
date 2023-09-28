//State machine idea from
/* List of Sources
* http://howtomakeanrpg.com/a/state-machines.html // Statemachine switching behaviour
* https://ajmmertens.medium.com/why-storing-state-machines-in-ecs-is-a-bad-idea-742de7a18e59
* https://www.richardlord.net/blog/ecs/finite-state-machines-with-ash.html // Statemachine how to add states
*/
/*
#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "tiny_ecs.hpp"

template<typename Component>
class StateMachineSystem
{
public:
    void Update(float elapsed);   
    std::unordered_map<std::string, ComponentContainer<Component>> _stateMap;

    void Add_MAP<Component>(std::string id, ComponentContainer<Component> state)
    {
        _stateMap.insert_or_assign(id, state);
    };
    void Remove_MAP(std::string id) 
    {
        _stateMap.erase(id);
    };
    void Clear_MAP()
    {
        _stateMap.clear();
    };

    //Add and remove state, checking state transitions
    void add(std::string id, Entity entity);
    void remove(std::string id, Entity entity);
    

};
/*
public interface IState
{
    void Update(float elapsed_ms);
    void HandleInput();
    void Enter(params object[] args);
    void Exit();
}

public class EmptyState : IState
{
    public void Update(float dt) {}
    public void HandleInput() {}
    public void Enter(params object[] args) {}
    public void Exit() {}
}

public class StateMachine
{
    Dictionary<string, IState> _stateDict
        = new Dictionary<string, IState>();
    IState _current = new EmptyState();

    public IState Current{ get { return _current; } }
    public void Add(string id, IState state) { _stateDict.Add(id, state); }
    public void Remove(string id) { _stateDict.Remove(id); }
    public void Clear() { _stateDict.Clear(); }


    public void Change(string id, params object[] args)
    {
        _current.Exit();
        IState next = _stateDict[id];
        next.Enter(args);
        _current = next;
    }

    public void Update(dt)
    {
        _current.Update(dt)
    }

    public void HandleInput()
    {
        _current.HandleInput();
    }
}
*/


