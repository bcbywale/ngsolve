#ifndef FILE_PROLONGATION
#define FILE_PROLONGATION

/*********************************************************************/
/* File:   prolongation.hh                                           */
/* Author: Joachim Schoeberl                                         */
/* Date:   20. Apr. 2000                                             */
/*********************************************************************/

namespace ngmg
{

  /** 
      Grid Transfer operators
  */
  class NGS_DLL_HEADER Prolongation
  {
  public:
    ///
    Prolongation();
    ///
    virtual ~Prolongation();
  
    ///
    virtual void Update () = 0;

    ///
    virtual SparseMatrix< double >* CreateProlongationMatrix( int finelevel ) const = 0;
    ///
    virtual void ProlongateInline (int finelevel, BaseVector & v) const = 0;
    ///
    virtual void RestrictInline (int finelevel, BaseVector & v) const = 0;

    virtual BitArray * GetInnerDofs () const { return 0; }
  };


  /**
     Standard Prolongation.
     Child nodes between 2 parent nodes.
  */
  // template <class TV>
  class LinearProlongation : public Prolongation
  {
    ///
    shared_ptr<MeshAccess> ma;
    ///
    const FESpace & space;
    ///
    Array<int> nvlevel;
  public:
    ///
    LinearProlongation(shared_ptr<MeshAccess> ama,
		       const FESpace & aspace)
      : ma(ama), space(aspace) { ; }
    ///
    LinearProlongation(const FESpace & aspace)
      : ma(aspace.GetMeshAccess()), space(aspace) { ; }
    
    virtual ~LinearProlongation(); 
    
                      ///
    virtual void Update () 
    { 
      if (ma->GetNLevels() > nvlevel.Size())
	nvlevel.Append (ma->GetNV());
    }


    /// 
    virtual SparseMatrix< double >* CreateProlongationMatrix( int finelevel ) const;


    ///
    virtual void ProlongateInline (int finelevel, BaseVector & v) const
    {
      int parents[2];

      int nc = nvlevel[finelevel-1];
      int nf = nvlevel[finelevel];
    
      //    FlatVector<TV> & fv = 
      //      dynamic_cast<VFlatVector<TV> &> (v).FV();
      //    FlatVector<TV> fv = 
      //      dynamic_cast<T_BaseVector<TV> &> (v).FV();

      FlatSysVector<> fv (v.Size(), v.EntrySize(), static_cast<double*>(v.Memory()));


      int i;
      for (i = nf; i < fv.Size(); i++)
	fv(i) = 0;

      for (i = nc; i < nf; i++)
	{
	  ma->GetParentNodes (i, parents);
	  fv(i) = 0.5 * (fv(parents[0]) + fv(parents[1]));
	}
    }


    ///
    virtual void RestrictInline (int finelevel, BaseVector & v) const
    {
      int parents[2];
      // int nc = space.GetNDofLevel (finelevel-1);
      // int nf = space.GetNDofLevel (finelevel);
      int nc = nvlevel[finelevel-1];
      int nf = nvlevel[finelevel];

      /*
	cout << "rest, h1, typeid(v) = " << typeid(v).name() << endl;
	cout << "nvlevel = " << nvlevel << ", level = " << finelevel << endl;
	cout << "nc = " << nc << ", nf = " << nf << endl;
	cout << "v.size = " << v.Size() << ", entrysize = " << v.EntrySize() << endl;
      */

      // FlatVector<TV> fv = dynamic_cast<T_BaseVector<TV> &> (v).FV();

      FlatSysVector<> fv (v.Size(), v.EntrySize(), static_cast<double*>(v.Memory()));

      int i;
      for (i = nf-1; i >= nc; i--)
	{
	  ma->GetParentNodes (i, parents);
	  fv(parents[0]) += 0.5 * fv(i);
	  fv(parents[1]) += 0.5 * fv(i);
	}

      for (i = nf; i < fv.Size(); i++)
	fv(i) = 0;  
    }
  };


  /*
  /// Prolongation for non-conforming P1 triangle.
  class NonConformingProlongation : public Prolongation
  {
    ///
    shared_ptr<MeshAccess> ma;
    ///
    const NonConformingFESpace & space;
  public:
    ///
    NonConformingProlongation(shared_ptr<MeshAccess> ama,
			      const NonConformingFESpace & aspace);
    ///
    virtual ~NonConformingProlongation();
  
    ///
    virtual void Update ();

    ///
    virtual SparseMatrix< double >* CreateProlongationMatrix( int finelevel ) const
    { return NULL; }
    ///
    virtual void ProlongateInline (int finelevel, BaseVector & v) const;
    ///
    virtual void RestrictInline (int finelevel, BaseVector & v) const;
  };
  */


  /// Piecewise constant prolongaton.
  class ElementProlongation : public Prolongation
  {
    ///
    shared_ptr<MeshAccess> ma;
    ///
    const ElementFESpace & space;
  public:
    ///
    ElementProlongation(const ElementFESpace & aspace)
      : ma(aspace.GetMeshAccess()), space(aspace) { ; }
    ///
    virtual ~ElementProlongation();
  
    ///
    virtual void Update ()
    { ; }

    ///
    virtual SparseMatrix< double >* CreateProlongationMatrix( int finelevel ) const
    { return NULL; }

    ///
    virtual void ProlongateInline (int finelevel, BaseVector & v) const
    {
      // FlatVector<TV> fv = dynamic_cast<T_BaseVector<TV> &> (v).FV();    

      FlatSysVector<> fv (v.Size(), v.EntrySize(), static_cast<double*>(v.Memory()));
    
      int nc = space.GetNDofLevel (finelevel-1);
      int nf = space.GetNDofLevel (finelevel);

      for (int i = nc; i < nf; i++)
	{
	  int parent = ma->GetParentElement (i);
	  fv(i) = fv(parent);
	}
    
      for (int i = nf; i < fv.Size(); i++)
	fv(i) = 0;
    }

    ///
    virtual void RestrictInline (int finelevel, BaseVector & v) const
    {
      //    FlatVector<TV> fv = dynamic_cast<T_BaseVector<TV> &> (v).FV();    

      FlatSysVector<> fv (v.Size(), v.EntrySize(), static_cast<double*>(v.Memory()));
    
      int nc = space.GetNDofLevel (finelevel-1);
      int nf = space.GetNDofLevel (finelevel);

      for (int i = nf-1; i >= nc; i--)
	{
	  int parent = ma->GetParentElement (i);
	  fv(parent) += fv(i);
	  fv(i) = 0;
	}
    }
  };



  /// Piecewise constant prolongation on boundary (implemented ?)
  class SurfaceElementProlongation : public Prolongation
  {
    ///
    shared_ptr<MeshAccess> ma;
    ///
    // const SurfaceElementFESpace & space;
  public:
    ///
    SurfaceElementProlongation(shared_ptr<MeshAccess> ama,
			       const SurfaceElementFESpace & aspace);
    ///
    virtual ~SurfaceElementProlongation();
  
    ///
    virtual void Update ();

    ///
    virtual SparseMatrix< double >* CreateProlongationMatrix( int finelevel ) const
    { return NULL; }
    ///
    virtual void ProlongateInline (int finelevel, BaseVector & v) const;
    ///
    virtual void RestrictInline (int finelevel, BaseVector & v) const;
  };



  /// Prolongation for edge-elements.
				     // template <class TV>
  class EdgeProlongation : public Prolongation
  {
    ///
    shared_ptr<MeshAccess> ma;
    ///
    const NedelecFESpace & space;
  public:
    ///
    EdgeProlongation(const NedelecFESpace & aspace)
      : ma(aspace.GetMeshAccess()), space(aspace) { ; }
    ///
    virtual ~EdgeProlongation() { ; }
  
    ///
    virtual void Update () { ; }

    ///
    virtual SparseMatrix< double >* CreateProlongationMatrix( int finelevel ) const
    { return NULL; }

    ///
    virtual void ProlongateInline (int finelevel, BaseVector & v) const
    {
      int nc = space.GetNDofLevel (finelevel-1);
      int nf = space.GetNDofLevel (finelevel);
      /*    
	    FlatVector<TV> & fv = 
	    dynamic_cast<VFlatVector<TV> &> (v).FV();
      */
      //    FlatVector<TV> fv = dynamic_cast<T_BaseVector<TV> &> (v).FV();
      FlatSysVector<> fv (v.Size(), v.EntrySize(), static_cast<double*>(v.Memory()));

      int i, k;

      for (i = nf; i < fv.Size(); i++)
	fv(i) = 0;

      for (k = 1; k <= 10; k++)
	for (i = nc; i < nf; i++)
	  {
	    int pa1 = space.ParentEdge1 (i);
	    int pa2 = space.ParentEdge2 (i);
	  
	    fv(i) = 0;
	    if (pa1 != -1)
	      {
		if (pa1 & 1)
		  fv(i) += 0.5 * fv(pa1/2);
		else
		  fv(i) -= 0.5 * fv(pa1/2);
	      }
	    if (pa2 != -1)
	      {
		if (pa2 & 1)
		  fv(i) += 0.5 * fv(pa2/2);
		else
		  fv(i) -= 0.5 * fv(pa2/2);
	      }
	  }

      for (i = 0; i < nf; i++)
	if (space.FineLevelOfEdge(i) < finelevel)
	  fv(i) = 0;
    }


    ///
    virtual void RestrictInline (int finelevel, BaseVector & v) const
    {
      int nc = space.GetNDofLevel (finelevel-1);
      int nf = space.GetNDofLevel (finelevel);

      //    FlatVector<TV> & fv = 
      //      dynamic_cast<VFlatVector<TV> &> (v).FV();
      //    FlatVector<TV> fv = dynamic_cast<T_BaseVector<TV> &> (v).FV();

      FlatSysVector<> fv (v.Size(), v.EntrySize(), static_cast<double*>(v.Memory()));

      for (int i = 0; i < nf; i++)
	if (space.FineLevelOfEdge(i) < finelevel)
	  fv(i) = 0;
	
      for (int k = 1; k <= 10; k++)
	for (int i = nf-1; i >= nc; i--)
	  {
	    int pa1 = space.ParentEdge1 (i);
	    int pa2 = space.ParentEdge2 (i);
	  
	    if (pa1 != -1)
	      {
		if (pa1 & 1)
		  fv(pa1/2) += 0.5 * fv(i);
		else
		  fv(pa1/2) -= 0.5 * fv(i);
	      }
	    if (pa2 != -1)
	      {
		if (pa2 & 1)
		  fv(pa2/2) += 0.5 * fv(i);
		else
		  fv(pa2/2) -= 0.5 * fv(i);
	      }
	    fv(i) = 0;
	  }

      for (int i = nf; i < fv.Size(); i++)
	fv(i) = 0;  
    }

    ///
    void ApplyGradient (int level, const BaseVector & pot, BaseVector & grad) const
    {
      cout << "apply grad" << endl;
    }
  };



    /// L2Ho prolongaton
  class L2HoProlongation : public Prolongation
  {
    ///
    shared_ptr<MeshAccess> ma;
    ///
    const Array<int> & first_dofs;
  public:
    ///
    L2HoProlongation(shared_ptr<MeshAccess> ama, const Array<int> & afirst_dofs);
    ///
    virtual ~L2HoProlongation()
    { ; }
    ///
    virtual void Update ()
	{ ; }

    ///
    virtual SparseMatrix< double >* CreateProlongationMatrix( int finelevel ) const
    { return NULL; }

    ///
    virtual void ProlongateInline (int finelevel, BaseVector & v) const;

    ///
    virtual void RestrictInline (int finelevel, BaseVector & v) const
    {
		cout << "RestrictInline not implemented for L2HoProlongation" << endl;
    }
 
  };


  /// Product space prolongation, combination of elementary prolongations 
  class CompoundProlongation : public Prolongation
  {
  protected:
    ///
    const CompoundFESpace * space;
    ///
    Array<shared_ptr<Prolongation>> prols;
  public:
    ///
    CompoundProlongation(const CompoundFESpace * aspace);
    ///
    CompoundProlongation(const CompoundFESpace * aspace,
			 Array<shared_ptr<Prolongation>> & aprols);
    ///
    virtual ~CompoundProlongation();
    // { ; }
  
    ///
    virtual void Update ();

    void AddProlongation (shared_ptr<Prolongation> prol)
    {
      prols.Append (prol);
    }

    ///
    virtual SparseMatrix< double >* CreateProlongationMatrix( int finelevel ) const
    { return NULL; }


    ///
    virtual void ProlongateInline (int finelevel, BaseVector & v) const;

    ///
    virtual void RestrictInline (int finelevel, BaseVector & v) const;
  };
}


#endif
