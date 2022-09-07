#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GRIT/hypergraph.h"
#include "GRIT/utility.h"


namespace py = pybind11;

void defineDataStructures(py::module &m) {

    py::class_<GRIT::Triplet> (m, "Triplet")
        .def(py::init<>())
        .def_readwrite("i", &GRIT::Triplet::i)
        .def_readwrite("j", &GRIT::Triplet::j)
        .def_readwrite("k", &GRIT::Triplet::k)
        .def("__eq__", &GRIT::Triplet::operator==)
        .def("__ne__", &GRIT::Triplet::operator!=);

    py::class_<GRIT::TriangleList> (m, "TriangleList")
        .def(py::init<size_t>())
        .def("get_triangle_number", &GRIT::TriangleList::getTriangleNumber)
        .def_static("load_from_binary", &GRIT::TriangleList::loadFromBinary);

    py::class_<GRIT::Hypergraph> (m, "Hypergraph")
        .def(py::init<size_t>())
        .def("add_edge", &GRIT::Hypergraph::addEdge)
        .def("add_triangle", [](GRIT::Hypergraph& self, size_t i, size_t j, size_t k) { self.addTriangle({i, j, k}); })
        .def("is_edge", &GRIT::Hypergraph::isEdge)
        .def("resize", &GRIT::Hypergraph::resize)
        .def("get_size", &GRIT::Hypergraph::getSize)
        .def("get_edge_number", &GRIT::Hypergraph::getEdgeNumber)
        .def("get_edge_multiplicity", &GRIT::Hypergraph::getEdgeMultiplicity)
        .def("get_edges_from", &GRIT::Hypergraph::getEdgesFrom)
        .def("get_highest_order_hyperedge_with", &GRIT::Hypergraph::getHighestOrderHyperedgeWith)
        .def("get_full_triangle_list", &GRIT::Hypergraph::getFullTriangleList)
        .def("get_triangle_list", &GRIT::Hypergraph::getTriangles)
        .def("get_triangle_number", &GRIT::Hypergraph::getTriangleNumber)
        .def("get_triangles_from", &GRIT::Hypergraph::getTrianglesFrom)
        .def("load_from_binary", &GRIT::Hypergraph::loadFromBinary)
        .def("write_to_binary", &GRIT::Hypergraph::writeToBinary)
        .def("write_to_csv", &GRIT::Hypergraph::writeToCSV)
        .def("get_copy", [](const GRIT::Hypergraph& self){ return GRIT::Hypergraph(self); });
}
