#include <boost/python.hpp>
#include "ApogeeAltaManager.h"
using namespace boost::python;

BOOST_PYTHON_MODULE(ApogeeAltaManagerPythonInterface)
{
    class_<ApogeeAltaManager>("ApogeeAltaManager", init<std::string>())
        .def("setUp", &ApogeeAltaManager::setUp)
        .def("run", &ApogeeAltaManager::run)
        .def("stop", &ApogeeAltaManager::stop)
    ;
}
