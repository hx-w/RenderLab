/**
 *  set command queue, invoke at each frame begin
 */

#ifndef RENDER_INVOKER_H
#define RENDER_INVOKER_H


#include <any>
#include <memory>
#include <vector>
#include <mutex>

#include <container/queue.hpp>


namespace RenderSpace {
    using ArgList = std::vector<std::any>;
    enum CommandType {
        AddDrawable,
        RemoveDrawable,
        UpdateDrawable,
        HideOrShowDrawable,

        Pick
    };
    
    class Command {
    public:
        Command() = delete;
        Command(CommandType type, ArgList&& args) : m_type(type), m_args(args) { }
        ~Command() = default;

        void invoke();

    private:
        CommandType m_type;
        ArgList m_args;
    };


    class CommandQueue {
    public:
        ~CommandQueue() = default;

        static std::shared_ptr<CommandQueue> get_instance();
        static void destroy();

        void push(Command&& cmd);

        void invoke();

    private:
        CommandQueue() = default;

    private:
        std::mutex m_mutex;
        fundamental::queue<Command> m_queue;

    private:
        static std::shared_ptr<CommandQueue> m_instance;
        static std::once_flag m_inited;
    };


    // static class method [Executor]
    class Executor {
    public:
        Executor() = delete;
        ~Executor() = delete;
    };
}




#endif
