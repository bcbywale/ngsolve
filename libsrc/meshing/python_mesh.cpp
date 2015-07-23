#ifdef NG_PYTHON

#include <boost/python.hpp>
#include <boost/python/slice.hpp>
#include <../general/ngpython.hpp>

#include <mystdlib.h>
#include "meshing.hpp"


using namespace netgen;
namespace bp = boost::python;

namespace netgen
{
  extern shared_ptr<NetgenGeometry> ng_geometry;
}


template <typename T, int BASE = 0, typename TIND = int>
void ExportArray ()
{
  string name = string("Array_") + typeid(T).name();
  bp::class_<Array<T,BASE,TIND>,boost::noncopyable>(name.c_str())
    .def ("__len__", &Array<T,BASE,TIND>::Size)
    .def ("__getitem__", 
          FunctionPointer ([](Array<T,BASE,TIND> & self, TIND i) -> T&
                           {
                             if (i < BASE || i >= BASE+self.Size())
                               bp::exec("raise IndexError()\n");
                             return self[i];
                           }),
          bp::return_value_policy<bp::reference_existing_object>())

    .def ("__iter__", 
          bp::range (FunctionPointer([](Array<T,BASE,TIND> & self) { return &self[BASE]; }),
                     FunctionPointer([](Array<T,BASE,TIND> & self) { return &self[BASE+self.Size()]; })))

    ;
}



DLL_HEADER void ExportNetgenMeshing() 
{
  
  ModuleScope module("meshing");

  bp::class_<PointIndex>("PointId", bp::init<int>())
    .def("__repr__", &ToString<PointIndex>)
    .def("__str__", &ToString<PointIndex>)
    .add_property("nr", &PointIndex::operator int)
    .def("__eq__" , FunctionPointer( [](PointIndex &self, PointIndex &other)
                  { return static_cast<int>(self)==static_cast<int>(other); }) )
    .def("__hash__" , FunctionPointer( [](PointIndex &self ) { return static_cast<int>(self); }) )
    ;

  bp::class_<ElementIndex>("ElementId3D", bp::init<int>())
    .def("__repr__", &ToString<ElementIndex>)
    .def("__str__", &ToString<ElementIndex>)
    .add_property("nr", &ElementIndex::operator int)
    .def("__eq__" , FunctionPointer( [](ElementIndex &self, ElementIndex &other)
                  { return static_cast<int>(self)==static_cast<int>(other); }) )
    .def("__hash__" , FunctionPointer( [](ElementIndex &self ) { return static_cast<int>(self); }) )
    ;


  bp::class_<SurfaceElementIndex>("ElementId2D", bp::init<int>())
    .def("__repr__", &ToString<SurfaceElementIndex>)
    .def("__str__", &ToString<SurfaceElementIndex>)
    .add_property("nr", &SurfaceElementIndex::operator int)
    .def("__eq__" , FunctionPointer( [](SurfaceElementIndex &self, SurfaceElementIndex &other)
                  { return static_cast<int>(self)==static_cast<int>(other); }) )
    .def("__hash__" , FunctionPointer( [](SurfaceElementIndex &self ) { return static_cast<int>(self); }) )
    ;

  bp::class_<SegmentIndex>("ElementId1D", bp::init<int>())
    .def("__repr__", &ToString<SegmentIndex>)
    .def("__str__", &ToString<SegmentIndex>)
    .add_property("nr", &SegmentIndex::operator int)
    .def("__eq__" , FunctionPointer( [](SegmentIndex &self, SegmentIndex &other)
                  { return static_cast<int>(self)==static_cast<int>(other); }) )
    .def("__hash__" , FunctionPointer( [](SegmentIndex &self ) { return static_cast<int>(self); }) )
    ;



  /*  
  bp::class_<Point<3>> ("Point")
    .def(bp::init<double,double,double>())
    ;
  */

  bp::class_<MeshPoint /* ,bp::bases<Point<3>> */ >("MeshPoint")
    .def(bp::init<Point<3>>())
    .def("__str__", &ToString<MeshPoint>)
    .def("__repr__", &ToString<MeshPoint>)
    .add_property("p", FunctionPointer([](const MeshPoint & self)
                                       {
                                         bp::list l;
                                         l.append ( self[0] );
                                         l.append ( self[1] );
                                         l.append ( self[2] );
                                         return bp::tuple(l);
                                       }))
    ;
  
  bp::class_<Element>("Element3D")
    .def("__init__", bp::make_constructor
         (FunctionPointer ([](int index, bp::list vertices)
                           {
                             Element * tmp = new Element(TET);
                             for (int i = 0; i < 4; i++)
                               (*tmp)[i] = bp::extract<PointIndex>(vertices[i]);
                             tmp->SetIndex(index);
                             return tmp;
                           }),
          bp::default_call_policies(),        // need it to use arguments
          (bp::arg("index")=1,bp::arg("vertices"))),
         "create volume element"
         )
    .def("__repr__", &ToString<Element>)
    .add_property("index", &Element::GetIndex, &Element::SetIndex)
    .add_property("vertices", 
                  FunctionPointer ([](const Element & self) -> bp::list
                                   {
                                     bp::list li;
                                     for (int i = 0; i < self.GetNV(); i++)
                                       li.append (self[i]);
                                     return li;
                                   }))
    ;

  bp::class_<Element2d>("Element2D")
    .def("__init__", bp::make_constructor
         (FunctionPointer ([](int index, bp::list vertices)
                           {
                             Element2d * tmp = new Element2d(TRIG);
                             for (int i = 0; i < 3; i++)
                               (*tmp)[i] = bp::extract<PointIndex>(vertices[i]);
                             tmp->SetIndex(index);
                             return tmp;
                           }),
          bp::default_call_policies(),        // need it to use arguments
          (bp::arg("index")=1,bp::arg("vertices"))),
         "create surface element"
         )
    .add_property("index", &Element2d::GetIndex, &Element2d::SetIndex)
    .add_property("vertices",
                  FunctionPointer([](const Element2d & self) -> bp::list
                                  {
                                    bp::list li;
                                    for (int i = 0; i < self.GetNV(); i++)
                                      li.append(self[i]);
                                    return li;
                                  }))
    ;

  bp::class_<Segment>("Element1D")
    .def("__init__", bp::make_constructor
         (FunctionPointer ([](bp::list vertices, bp::list surfaces)
                           {
                             Segment * tmp = new Segment;
                             for (int i = 0; i < 2; i++)
                               (*tmp)[i] = bp::extract<PointIndex>(vertices[i]);

                             tmp->surfnr1 = bp::extract<int>(surfaces[0]);
                             tmp->surfnr2 = bp::extract<int>(surfaces[1]);
                             return tmp;
                           }),
          bp::default_call_policies(),      
          (bp::arg("vertices"),bp::arg("surfaces"))),
         "create segment element"
         )
    .def("__repr__", &ToString<Element>)
    .add_property("vertices", 
                  FunctionPointer ([](const Segment & self) -> bp::list
                                   {
                                     bp::list li;
                                     for (int i = 0; i < 2; i++)
                                       li.append (self[i]);
                                     return li;
                                   }))
    .add_property("surfaces", 
                  FunctionPointer ([](const Segment & self) -> bp::list
                                   {
                                     bp::list li;
                                     li.append (self.surfnr1);
                                     li.append (self.surfnr2);
                                     return li;
                                   }))
    ;




  bp::class_<FaceDescriptor>("FaceDescriptor")
    .def(bp::init<const FaceDescriptor&>())
    .def("__str__", &ToString<FaceDescriptor>)
    .def("__repr__", &ToString<FaceDescriptor>)
    .add_property("surfnr", &FaceDescriptor::SurfNr, &FaceDescriptor::SetSurfNr)
    ;

  

  ExportArray<Element>();
  ExportArray<Element2d>();
  ExportArray<Segment>();
  ExportArray<MeshPoint,PointIndex::BASE,PointIndex>();
  ExportArray<FaceDescriptor>();
  ;
  
  
  bp::class_<Mesh,shared_ptr<Mesh>,boost::noncopyable>("Mesh", bp::no_init)
    .def(bp::init<>("create empty mesh"))
    .def("__str__", &ToString<Mesh>)
    .def("Load",  FunctionPointer 
	 ([](Mesh & self, const string & filename)
	  {
	    ifstream input(filename);
	    self.Load(input);
	    for (int i = 0; i < geometryregister.Size(); i++)
	      {
		NetgenGeometry * hgeom = geometryregister[i]->LoadFromMeshFile (input);
		if (hgeom)
		  {
		    ng_geometry.reset (hgeom);
		    break;
		  }
	      }
	  }))
    // static_cast<void(Mesh::*)(const string & name)>(&Mesh::Load))
    .def("Save", static_cast<void(Mesh::*)(const string & name)const>(&Mesh::Save))

    .def("Elements3D", 
         static_cast<Array<Element>&(Mesh::*)()> (&Mesh::VolumeElements),
         bp::return_value_policy<bp::reference_existing_object>())

    .def("Elements2D", 
         static_cast<Array<Element2d>&(Mesh::*)()> (&Mesh::SurfaceElements),
         bp::return_value_policy<bp::reference_existing_object>())

    .def("Elements1D", 
         static_cast<Array<Segment>&(Mesh::*)()> (&Mesh::LineSegments),
         bp::return_value_policy<bp::reference_existing_object>())


    .def("Points", 
         static_cast<Mesh::T_POINTS&(Mesh::*)()> (&Mesh::Points),
         bp::return_value_policy<bp::reference_existing_object>())

    .def("FaceDescriptor", static_cast<FaceDescriptor&(Mesh::*)(int)> (&Mesh::GetFaceDescriptor),
         bp::return_value_policy<bp::reference_existing_object>())

    .def("__getitem__", FunctionPointer ([](const Mesh & self, PointIndex pi)
                                         {
                                           return self[pi];
                                         }))

    .def ("Add", FunctionPointer ([](Mesh & self, MeshPoint p)
                                  {
                                    return self.AddPoint (Point3d(p));
                                  }))

    .def ("Add", FunctionPointer ([](Mesh & self, const Element & el)
                                  {
                                    return self.AddVolumeElement (el);
                                  }))

    .def ("Add", FunctionPointer ([](Mesh & self, const Element2d & el)
                                  {
                                    return self.AddSurfaceElement (el);
                                  }))

    .def ("Add", FunctionPointer ([](Mesh & self, const Segment & el)
                                  {
                                    return self.AddSegment (el);
                                  }))

    .def ("Add", FunctionPointer ([](Mesh & self, const FaceDescriptor & fd)
                                  {
                                    return self.AddFaceDescriptor (fd);
                                  }))

    /*
    .def("__init__", bp::make_constructor
         (FunctionPointer ([]()
                           {
                             cout << "create new mesh" << endl;
                             auto tmp = make_shared<Mesh>();
                             return tmp;
                           })),
         "create empty mesh"
      )
    */
    .def ("BoundaryLayer", FunctionPointer 
          ([](Mesh & self, int bc, double thickness, int volnr, string material)
           {
             BoundaryLayerParameters blp;

             for (int i = 1; i <= self.GetNFD(); i++)
               if (self.GetFaceDescriptor(i).BCProperty() == bc)
                   blp.surfid.Append (i);

             cout << "add layer at surfaces: " << blp.surfid << endl;

             blp.prismlayers = 1;
             blp.hfirst = thickness;
             blp.growthfactor = 1.0;

             // find max domain nr
             int maxind = 0;
             for (ElementIndex ei = 0; ei < self.GetNE(); ei++)
               maxind = max (maxind, self[ei].GetIndex());
             cout << "maxind = " << maxind << endl;
             self.SetMaterial (maxind+1, material.c_str());
             blp.new_matnr = maxind+1;
             blp.bulk_matnr = volnr;
             GenerateBoundaryLayer (self, blp);
           }
           ))
                                            
    ;
  

  typedef MeshingParameters MP;
  bp::class_<MP> ("MeshingParameters", bp::init<>())
    .def("__init__", bp::make_constructor
         (FunctionPointer ([](double maxh)
                           {
                             auto tmp = new MeshingParameters;
                             tmp->maxh = maxh;
                             return tmp;
                           }),
          bp::default_call_policies(),        // need it to use arguments
          (bp::arg("maxh")=1000)),
         "create meshing parameters"
         )
    .def("__str__", &ToString<MP>)
    .add_property("maxh", 
                  FunctionPointer ([](const MP & mp ) { return mp.maxh; }),
                  FunctionPointer ([](MP & mp, double maxh) { return mp.maxh = maxh; }))
                  
    ;

  bp::def("SetTestoutFile", FunctionPointer ([] (const string & filename)
                                             {
                                               delete testout;
                                               testout = new ofstream (filename);
                                             }));

  bp::def("SetMessageImportance", FunctionPointer ([] (int importance)
                                                   {
                                                     int old = printmessage_importance;
                                                     printmessage_importance = importance;
                                                     return old;
                                                   }));
}



BOOST_PYTHON_MODULE(libmesh) {
  ExportNetgenMeshing();
}



#endif



