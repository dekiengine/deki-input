#include "DekiInput.h"
#include <deki/LogSystem.h>

namespace DekiInput
{

// Static member definitions
std::map<std::string, std::unique_ptr<IDekiInput>> DekiInput::s_ActiveInputs;
std::vector<InputEventCallback> DekiInput::s_GlobalCallbacks;
bool DekiInput::s_Initialized = false;
bool DekiInput::s_ShouldExit = false;
bool DekiInput::s_InSession = false;
std::set<std::string> DekiInput::s_HostInputs;
std::set<size_t> DekiInput::s_SessionCallbacks;

bool DekiInput::SetInput(std::unique_ptr<IDekiInput> input, const std::string& name)
{
    if (!input)
    {
        DEKI_LOG_ERROR("DekiInput::SetInput: null input");
        return false;
    }

    if (s_ActiveInputs.find(name) != s_ActiveInputs.end())
    {
        DEKI_LOG_INTERNAL("Input package '%s' already registered", name.c_str());
        return true;
    }

    // Register internal callback to distribute events, tagged with the name
    // the source is known by. The map's key lives as long as the entry.
    auto entry = s_ActiveInputs.emplace(name, std::move(input)).first;
    const char* source = entry->first.c_str();
    entry->second->RegisterEventCallback(
        [source](const InputEvent& event)
        {
            InputEvent tagged = event;
            tagged.source = source;
            DistributeEvent(tagged);
        });
    s_Initialized = true;

    DEKI_LOG_INTERNAL("DekiInput: Input '%s' set", name.c_str());
    return true;
}

bool DekiInput::RemoveInput(const std::string& name)
{
    auto it = s_ActiveInputs.find(name);
    if (it == s_ActiveInputs.end())
    {
        return false;
    }
    if (it->second)
    {
        it->second->Shutdown();
    }
    s_ActiveInputs.erase(it);
    return true;
}

void DekiInput::BeginSession()
{
    // Once: a scene opened during a session does not start another.
    if (s_InSession)
    {
        return;
    }
    s_InSession = true;
    s_HostInputs.clear();
    for (const auto& entry : s_ActiveInputs)
    {
        s_HostInputs.insert(entry.first);
    }
    s_SessionCallbacks.clear();
}

void DekiInput::EndSession()
{
    if (!s_InSession)
    {
        return;
    }
    s_InSession = false;
    const std::set<size_t> callbacks = s_SessionCallbacks;
    for (size_t id : callbacks)
    {
        UnregisterEventCallback(id);
    }
    std::vector<std::string> added;
    for (const auto& entry : s_ActiveInputs)
    {
        if (s_HostInputs.count(entry.first) == 0)
        {
            added.push_back(entry.first);
        }
    }
    for (const std::string& name : added)
    {
        DEKI_LOG_INTERNAL("DekiInput: removing '%s', added while the game ran", name.c_str());
        RemoveInput(name);
    }
    s_HostInputs.clear();
}

void DekiInput::Shutdown()
{
    if (!s_Initialized)
    {
        return;
    }

    for (auto& [name, input] : s_ActiveInputs)
    {
        if (input)
        {
            input->Shutdown();
        }
    }

    s_ActiveInputs.clear();
    s_GlobalCallbacks.clear();
    s_Initialized = false;
    s_ShouldExit = false;

    DEKI_LOG_INTERNAL("DekiInput shutdown");
}

IDekiInput* DekiInput::GetInput(const std::string& name)
{
    auto it = s_ActiveInputs.find(name);
    if (it != s_ActiveInputs.end())
    {
        return it->second.get();
    }
    return nullptr;
}

size_t DekiInput::RegisterEventCallback(const InputEventCallback& callback)
{
    // A slot a removed callback left, else a new one: ids stay valid, as
    // they are positions.
    size_t id = s_GlobalCallbacks.size();
    for (size_t i = 0; i < s_GlobalCallbacks.size(); ++i)
    {
        if (!s_GlobalCallbacks[i])
        {
            id = i;
            break;
        }
    }
    if (id == s_GlobalCallbacks.size())
    {
        s_GlobalCallbacks.push_back(callback);
    }
    else
    {
        s_GlobalCallbacks[id] = callback;
    }
    if (s_InSession)
    {
        s_SessionCallbacks.insert(id);
    }
    return id;
}

void DekiInput::UnregisterEventCallback(size_t id)
{
    if (id < s_GlobalCallbacks.size())
    {
        s_GlobalCallbacks[id] = nullptr;
    }
    // Its slot may be reused, by the host as much as by the game.
    s_SessionCallbacks.erase(id);
}

void DekiInput::ClearEventCallbacks()
{
    s_GlobalCallbacks.clear();
    s_SessionCallbacks.clear();
}

void DekiInput::Update()
{
    if (!s_Initialized)
    {
        return;
    }

    for (auto& [name, input] : s_ActiveInputs)
    {
        if (input && input->IsInitialized())
        {
            input->Update();
        }
    }
}

bool DekiInput::GetPointerPosition(int32_t* x, int32_t* y)
{
    if (!s_Initialized || !x || !y)
    {
        return false;
    }

    // Try to get position from first available input
    for (auto& [name, input] : s_ActiveInputs)
    {
        if (input && input->IsInitialized())
        {
            if (input->GetPointerPosition(x, y))
            {
                return true;
            }
        }
    }

    return false;
}

bool DekiInput::IsKeyPressed(uint32_t key)
{
    if (!s_Initialized)
    {
        return false;
    }

    // Check if key is pressed in any active input
    for (auto& [name, input] : s_ActiveInputs)
    {
        if (input && input->IsInitialized())
        {
            if (input->IsKeyPressed(key))
            {
                return true;
            }
        }
    }

    return false;
}

bool DekiInput::ShouldExit()
{
    return s_ShouldExit;
}

void DekiInput::SetShouldExit(bool exit)
{
    s_ShouldExit = exit;
}

std::vector<std::string> DekiInput::GetActiveInputs()
{
    std::vector<std::string> names;
    for (const auto& [name, input] : s_ActiveInputs)
    {
        names.push_back(name);
    }
    return names;
}

void DekiInput::DistributeEvent(const InputEvent& event)
{
    if (event.type == InputEventType::AppQuit)
    {
        s_ShouldExit = true;
        DEKI_LOG_INTERNAL("DekiInput: AppQuit event received");
    }

    // By position, as a callback may add or remove callbacks; each is called
    // through a copy, so one that removes itself is not destroyed while it
    // runs.
    for (size_t i = 0; i < s_GlobalCallbacks.size(); ++i)
    {
        const InputEventCallback callback = s_GlobalCallbacks[i];
        if (callback)
        {
            callback(event);
        }
    }
}

}  // namespace DekiInput
