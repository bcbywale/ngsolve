#ifndef FILE_HCURLHDIVFES
#define FILE_HCURLHDIVFES

/*********************************************************************/
/* File:   hcurlhdivfes.hh                                           */
/* Author: Joachim Schoeberl                                         */
/* Date:   12. Jan. 2002                                             */
/*********************************************************************/

namespace ngcomp
{

/*
   Finite Element Space
*/


/// Lowest order Nedelec space (edge elements)
class NGS_DLL_HEADER NedelecFESpace : public FESpace
{
  ///
  Array<ngstd::INT<2> > edgepoints;
  ///
  Array<INT<2> > parentedges;
  ///
  Array<short int> finelevelofedge;
  ///
  Array<int> nelevel;

protected:
  bool discontinuous;
  
public:
  ///
  NedelecFESpace (shared_ptr<MeshAccess> ama, const Flags & flags, bool parseflags=false);
  ///
  virtual ~NedelecFESpace ();

  ///
  virtual const char * GetType() 
    { return "Nedelec"; }

  static shared_ptr<FESpace> Create (shared_ptr<MeshAccess> ma, const Flags & flags);

  ///
  virtual void Update(LocalHeap & lh);
  ///
  virtual void DoArchive (Archive & archive);
  /// 
  virtual void UpdateCouplingDofArray();

  ///
  virtual size_t GetNDof () const throw();
  ///
  virtual size_t GetNDofLevel (int level) const;

  /// 
  virtual void GetDofRanges (ElementId ei, Array<IntRange> & dranges) const;

  ///
  virtual void GetDofNrs (ElementId ei, Array<int> & dnums) const;
  ///
  int EdgePoint1 (int ednr) const { return edgepoints[ednr][0]; }
  ///
  int EdgePoint2 (int ednr) const { return edgepoints[ednr][1]; }

  ///
  int ParentEdge1 (int ednr) const { return parentedges[ednr][0]; }
  ///
  int ParentEdge2 (int ednr) const { return parentedges[ednr][1]; }

  ///
  int FineLevelOfEdge (int ednr) const { return finelevelofedge[ednr]; }

  enum { SB_AFW, SB_HIPTMAIR, SB_POTENTIAL, SB_JAC };
  ///
  virtual shared_ptr<Table<int>> CreateSmoothingBlocks (int type = 0) const;
  virtual shared_ptr<Table<int>> CreateSmoothingBlocks (const Flags & precflags) const;

  SparseMatrix<double> * CreateGradient() const;

  template <class T>
    NGS_DLL_HEADER void TransformMat (ElementId ei, 
                                      SliceMatrix<T> mat, TRANSFORM_TYPE tt) const;
  
  template <class T>
    NGS_DLL_HEADER void TransformVec (ElementId ei, 
                                      SliceVector<T> vec, TRANSFORM_TYPE tt) const;
  
  
  virtual void VTransformMR (ElementId ei, 
			     SliceMatrix<double> mat, TRANSFORM_TYPE tt) const 
  {
    TransformMat (ei, mat, tt);
  }

  virtual void VTransformMC (ElementId ei, 
			     SliceMatrix<Complex> mat, TRANSFORM_TYPE tt) const
  {
    TransformMat (ei, mat, tt);
  }

  virtual void VTransformVR (ElementId ei, 
			     SliceVector<double> vec, TRANSFORM_TYPE tt) const 
  {
    TransformVec (ei, vec, tt);
  }

  virtual void VTransformVC (ElementId ei, 
			     SliceVector<Complex> vec, TRANSFORM_TYPE tt) const 
  {
    TransformVec (ei, vec, tt);
  }




  virtual string GetClassName () const
  {
    return "NedelecFESpace";
  }


  virtual void GetVertexDofNrs (int vnr, Array<int> & dnums) const;
  virtual void GetEdgeDofNrs (int ednr, Array<int> & dnums) const;
  virtual void GetFaceDofNrs (int fanr, Array<int> & dnums) const;
  virtual void GetInnerDofNrs (int elnr, Array<int> & dnums) const;
};




///
class NGS_DLL_HEADER NedelecFESpace2 : public FESpace
{
public:
  ///
  //  enum NEDELEC_TYPE { N1, BDM1, BDM1A, N2, BDM2A, BDM2 };

private:
  /// order in z-direction
  int zorder;
  ///
  Array<short int> gradientedge;
  ///
  Array<short int> gradientface;

  ///
  int ned;
  ///
  int nfa;
  ///
  int nel;
  ///
  int n_edge_dofs;
  int n_z_edge_dofs;
  int n_plane_edge_dofs;
  int n_trig_face_dofs;
  int n_quad_face_dofs;
  int n_tet_el_dofs;
  int n_prism_el_dofs;
  int n_prism_nograd_el_dofs;
  int n_pyramid_el_dofs;
  ///
  Array<int> first_face_dof;
  Array<int> first_el_dof;
  ///
  BitArray gradientdomains;
  ///
  BitArray gradientboundaries;
  ///
  Array<int> ndlevel;

  FiniteElement * curltet;
  FiniteElement * curlprism;
  FiniteElement * curlpyramid;

public:
  ///
  NedelecFESpace2 (shared_ptr<MeshAccess> ama, const Flags & flags, bool parseflags=false);
  ///
  ~NedelecFESpace2 ();

  ///
  virtual const char * GetType() 
    { return "Nedelec2"; }

  virtual string GetClassName () const
  {
    return "NedelecFESpace2";
  }


  ///
  virtual void Update(LocalHeap & lh);

  ///
  virtual size_t GetNDof () const throw();
  ///
  virtual size_t GetNDofLevel (int level) const;

  ///
  virtual void GetDofNrs (ElementId ei, Array<int> & dnums) const;

  using FESpace::GetFE;
  virtual const FiniteElement & GetFE (int elnr, LocalHeap & lh) const;

  ///
  void SetGradientDomains (const BitArray & adoms);
  ///
  void SetGradientBoundaries (const BitArray & abnds);

  ///
  void GetTransformation (ELEMENT_TYPE eltype, 
			  int elnr,
			  const Array<int> & eorient,
			  const Array<int> & forient,
			  FlatVector<double> & fac) const;
			  

  template <class T>
    NGS_DLL_HEADER void TransformMat (ElementId ei, 
                                      SliceMatrix<T> mat, TRANSFORM_TYPE tt) const;
  
  template <class T>
    NGS_DLL_HEADER void TransformVec (ElementId ei, 
                                      SliceVector<T> vec, TRANSFORM_TYPE tt) const;


  virtual void VTransformMR (ElementId ei, 
			     SliceMatrix<double> mat, TRANSFORM_TYPE tt) const 
  {
    TransformMat (ei, mat, tt);
  }

  virtual void VTransformMC (ElementId ei, 
			     SliceMatrix<Complex> mat, TRANSFORM_TYPE tt) const
  {
    TransformMat (ei, mat, tt);
  }



  virtual void VTransformVR (ElementId ei, 
			     SliceVector<double> vec, TRANSFORM_TYPE tt) const 
  {
    TransformVec (ei, vec, tt);
  }

  virtual void VTransformVC (ElementId ei, 
			     SliceVector<Complex> vec, TRANSFORM_TYPE tt) const 
  {
    TransformVec (ei, vec, tt);
  }

  ///
  virtual void LockSomeDofs (BaseMatrix & mat) const;
  ///
  // virtual Table<int> * CreateSmoothingBlocks (int type = 0) const;
  virtual shared_ptr<Table<int>> CreateSmoothingBlocks (const Flags & precflags) const;
  /// for anisotropic plane smoothing
  virtual BitArray * CreateIntermediatePlanes (int type = 0) const;

  ///
  SparseMatrix<double> * CreateGradient() const;

  
  virtual Array<int> * CreateDirectSolverClusters (const Flags & flags) const;


  virtual void GetVertexDofNrs (int vnr, Array<int> & dnums) const;
  virtual void GetEdgeDofNrs (int ednr, Array<int> & dnums) const;
  virtual void GetFaceDofNrs (int fanr, Array<int> & dnums) const;
  virtual void GetInnerDofNrs (int elnr, Array<int> & dnums) const;

//  void AddGradient (double fac, const BaseVector & pot, BaseVector & grad) const;
//  void ApplyGradientT (const BaseVector & gradt, BaseVector & pott) const;
};

}


#endif
