//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef APPLICATION_H
#define APPLICATION_H

#include "utils/NoCopy.h"
#include <cstddef> //For size_t.
#include <cassert>
#include <string>
#include <vector>

#include "FffProcessor.h"
#include "ccglobal/tracer.h"

namespace cura52
{
class Communication;
class Slice;
class ThreadPool;

/*!
 * A singleton class that serves as the starting point for all slicing.
 *
 * The application provides a starting point for the slicing engine. It
 * maintains communication with other applications and uses that to schedule
 * slices.
 */
class Application : NoCopy
{
public:
    FffProcessor processor;
    /*
     * \brief The communication currently in use.
     *
     * This may be set to ``nullptr`` during the initialisation of the program,
     * while the correct communication class has not yet been chosen because the
     * command line arguments have not yet been parsed. In general though you
     * can assume that it is safe to access this without checking whether it is
     * initialised.
     */
    Communication* communication = nullptr;

    /*
     * \brief The slice that is currently ongoing.
     *
     * If no slice has started yet, this will be a nullptr.
     */
    Slice* current_slice = nullptr;

    /*!
     * \brief ThreadPool with lifetime tied to Application
     */
    ThreadPool* thread_pool = nullptr;

    /*!
     * Gets the instance of this application class.
     */
    static Application& getInstance();

    /*!
     * \brief Print to the stderr channel what the original call to the executable was.
     */
    void printCall(int argc, const char** argv) const;

    /*!
     * \brief Print to the stderr channel how to use CuraEngine.
     */
    void printHelp() const;

    /*!
     * \brief Starts the application.
     *
     * It will start by parsing the command line arguments to see what it must
     * be doing.
     *
     * This function can only be called once, because it has side-effects on
     * static fields across the application.
     * \param argc The number of arguments provided to the application.
     * \param argv The arguments provided to the application.
     */
    void run(int argc, const char** argv, ccglobal::Tracer* tracer = nullptr);

    void runCommulication(Communication* communication);
    void releaseCommulication();
    /*!
     * \brief Start the global thread pool.
     *
     * If `nworkers` <= 0 and there is no pre-existing thread pool, a thread
     * pool with hardware_concurrency() workers is initialized.
     * The thread pool is restarted when the number of thread differs from
     * previous invocations.
     *
     * \param nworkers The number of workers (including the main thread) that are ran.
     */
    void startThreadPool(int nworkers = 0);

    void setSliceCommunication(Communication* ptr);
protected:
#ifdef ARCUS
    /*!
     * \brief Connect using libArcus to a socket.
     * \param argc The number of arguments provided to the application.
     * \param argv The arguments provided to the application.
     */
    void connect();
#endif //ARCUS

    /*!
     * \brief Print the header and license to the stderr channel.
     */
    void printLicense() const;

    /*!
     * \brief Start slicing.
     * \param argc The number of arguments provided to the application.
     * \param argv The arguments provided to the application.
     */
    void slice(ccglobal::Tracer* tracer = nullptr);

private:
    std::vector<std::string> m_args;

    /*!
     * \brief Constructs a new Application instance.
     *
     * You cannot call this because this goes via the getInstance() function.
     */
    Application();

    /*!
     * \brief Destroys the Application instance.
     *
     * This destroys the Communication instance along with it.
     */
    ~Application();
};

} //Cura namespace.

#endif //APPLICATION_H