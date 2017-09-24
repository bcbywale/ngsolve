#include <comp.hpp>

#include "../fem/hdivdivfe.hpp"
#include "hdivdivsurfacespace.hpp"
// #include "astrid.hpp"


namespace ngcomp
{

  template<int D>
  class DiffOpIdHDivDiv: public DiffOp<DiffOpIdHDivDiv<D> >
  {
  public:
    enum { DIM = 1 };
    enum { DIM_SPACE = D };
    enum { DIM_ELEMENT = D };
    enum { DIM_DMAT = D*D };
    enum { DIFFORDER = 0 };
    enum { DIM_STRESS = D*D };

    static Array<int> GetDimensions() { return Array<int> ({D,D}); }

    template <typename FEL,typename SIP>
    static void GenerateMatrix(const FEL & bfel,const SIP & mip,
      SliceMatrix<double,ColMajor> mat,LocalHeap & lh)
    {
      const HDivDivFiniteElement<D> & fel =
        dynamic_cast<const HDivDivFiniteElement<D>&> (bfel);
      fel.CalcMappedShape_Matrix (mip,Trans(mat));
    }

    template <typename FEL,typename SIP,typename MAT>
    static void GenerateMatrix(const FEL & bfel,const SIP & sip,
      MAT & mat,LocalHeap & lh)
    {
      const HDivDivFiniteElement<D> & fel =
        dynamic_cast<const HDivDivFiniteElement<D>&> (bfel);
      int nd = fel.GetNDof();
      FlatMatrix<> shape(nd,DIM_DMAT,lh);
      fel.CalcMappedShape_Matrix(sip,shape);
      for(int i=0; i<nd; i++)
        for(int j = 0; j <DIM_DMAT; j++)
          mat(j,i) = shape(i,j);

    }
  };

  

  /// Identity operator, Piola transformation
  template <int D, typename FEL = HDivDivFiniteElement<D-1> >
  class DiffOpIdHDivDivSurface : public DiffOp<DiffOpIdHDivDivSurface<D, FEL> >
  {
  public:
    enum { DIM = 1 };
    enum { DIM_SPACE = D };
    enum { DIM_ELEMENT = D-1 };
    enum { DIM_DMAT = D*D };
    enum { DIFFORDER = 0 };

    static const FEL & Cast(const FiniteElement & fel)
    {
      return static_cast<const FEL&> (fel);
    }

    static Array<int> GetDimensions() { return Array<int> ({D,D}); }
    
    template <typename AFEL, typename MIP, typename MAT>
    static void GenerateMatrix (const AFEL & bfel, const MIP & sip,
                                MAT & mat, LocalHeap & lh)
    {
      HeapReset hr(lh);
      const HDivDivFiniteElement<2> & fel = 
        dynamic_cast<const HDivDivFiniteElement<2>&> (bfel);
      
      int nd = fel.GetNDof();
      
      Mat<3,2> jac = sip.GetJacobian();
      double det = fabs (sip.GetJacobiDet());

      FlatMatrix<> shape(nd, 3, lh);
      fel.CalcShape (sip.IP(), shape);
      
      Mat<3,9> trans;
      for (int i = 0; i < 3; i++)
        {
          Mat<2> sigma_ref;
          sigma_ref = 0.0;
          switch (i)
            {
            case 0: sigma_ref(0,0) = 1.0; break;
            case 1: sigma_ref(1,1) = 1.0; break;
            case 2: sigma_ref(0,1) = sigma_ref(1,0) = 1.0; break;
            }
          auto hm = jac * sigma_ref;
          auto sigma = hm * Trans(jac);
          sigma *= (1.0 / sqr(det));
          
          trans ( i, 0 ) = sigma(0,0);
          trans ( i, 1 ) = sigma(0,1);
          trans ( i, 2 ) = sigma(0,2);
          trans ( i, 3 ) = sigma(1,0);
          trans ( i, 4 ) = sigma(1,1);
          trans ( i, 5 ) = sigma(1,2);
          trans ( i, 6 ) = sigma(2,0);
          trans ( i, 7 ) = sigma(2,1);
          trans ( i, 8 ) = sigma(2,2);
        }
      mat = Trans(trans) * Trans (shape);
    }
  };

  template <int D, typename FEL = HDivDivFiniteElement<D-1> >
  class DiffOpDivHDivDivSurface : public DiffOp<DiffOpDivHDivDivSurface<D, FEL> >
  {
  
  public:
    enum { DIM = 1 };
    enum { DIM_SPACE = D };
    enum { DIM_ELEMENT = D-1 };
    enum { DIM_DMAT = D };
    enum { DIFFORDER = 1 };
    enum { DIM_STRESS = (D*(D+1))/2 };

    template <typename AFEL, typename MIP, typename MAT>
    static void GenerateMatrix (const AFEL & bfel, const MIP & sip,
                                MAT & mat, LocalHeap & lh)
    {
      const HDivDivFiniteElement<2> & fel = 
        dynamic_cast<const HDivDivFiniteElement<2>&> (bfel);
    
      int nd = fel.GetNDof();

      FlatMatrix<> div_shape(nd, 2, lh);
      fel.CalcDivShape (sip.IP(), div_shape);

      FlatMatrix<> shape(nd, 3, lh);
      fel.CalcShape (sip.IP(), shape);

      Mat<3,2> jac = sip.GetJacobian();
      double det = fabs (sip.GetJacobiDet());
      Mat<3,2> sjac = (1.0/(det*det)) * jac;
      Mat<2,3> inv_jac = sip.GetJacobianInverse();

      mat = (sjac) * Trans (div_shape);
   
      //for non-curved elements, divergence transformation is finished, otherwise derivatives of Jacobian have to be computed...
      if (!sip.GetTransformation().IsCurvedElement())
        {
          return;
        }


      Mat<2,2> hesse[3];
      sip.CalcHesse (hesse[0], hesse[1], hesse[2]);
      Mat<3,2,AutoDiff<2> > fad;
      for (int i = 0; i < 3; i++)
        {
          for (int j = 0; j < 2; j++)
            {
              fad(i,j).Value() = jac(i,j);
              for (int k = 0; k < 2; k++)
                fad(i,j).DValue(k) = hesse[i](j,k);
            }
        }

      Vec<3, AutoDiff<2>> n = Cross(Vec<3,AutoDiff<2>>(fad.Col(0)),Vec<3,AutoDiff<2>>(fad.Col(1)));
      AutoDiff<2> ad_det = sqrt(n(0)*n(0)+n(1)*n(1)+n(2)*n(2));//L2Norm2(n);

      if (ad_det.Value() < 0.0)
        {
          // 	cout << "neg det" << endl;
          ad_det *= -1;
        }    

      AutoDiff<2> iad_det = 1.0 / ad_det;
      fad *= iad_det;

      for (int i = 0; i < nd; i++)
        {
          Mat<2> sigma_ref;

          sigma_ref(0,0) = shape(i, 0);
          sigma_ref(1,1) = shape(i, 1);
          sigma_ref(0,1) = sigma_ref(1,0) = shape(i, 2);
	 
	
          Vec<3> hv2;
          hv2 = 0.0;
          for (int j = 0; j < 2; j++)
            for (int k = 0; k < 3; k++)
              for (int l = 0; l < 2; l++)
                hv2(k) += fad(k,l).DValue(j) * sigma_ref(l,j);

          hv2 *= iad_det.Value();

          for ( int j = 0; j < 2; j++ )
            for ( int k = 0; k < 2; k++ )
              for ( int l = 0; l < 2; l++ )
                for ( int m = 0; m < 2; m++ )
                  for ( int n = 0; n < 3; n++ )
                    hv2(n) += inv_jac(m,k) *fad(n,j).Value() * sigma_ref(j,l) * fad(k,l).DValue(m);

          for ( int j = 0; j < 3; j++)
            mat(j,i) += hv2(j);
        }
    }
  
  };


  template <int D>
  class HDivDivSurfaceMassIntegrator 
    : public T_BDBIntegrator<DiffOpIdHDivDivSurface<D>, DiagDMat<DiffOpIdHDivDivSurface<D>::DIM_DMAT>, FiniteElement>
  {
    typedef T_BDBIntegrator<DiffOpIdHDivDivSurface<D>, DiagDMat<DiffOpIdHDivDivSurface<D>::DIM_DMAT>, FiniteElement> BASE;
  public:
    using  T_BDBIntegrator<DiffOpIdHDivDivSurface<D>, DiagDMat<DiffOpIdHDivDivSurface<D>::DIM_DMAT>, FiniteElement>::T_BDBIntegrator;

    virtual string Name () const { return "HDivDivSurface-Mass"; }
  };


  HDivDivSurfaceSpace::HDivDivSurfaceSpace(shared_ptr<MeshAccess> ama,
                                           const Flags & aflags, bool parseflags)
    : FESpace(ama, aflags)
  {
    order = aflags.GetNumFlag("order", 0);
    DefineNumFlag("discontinuous");
		
    noncontinuous = int(aflags.GetNumFlag("discontinuous", 0));

    // for the dimension ..
    evaluator[VOL] = make_shared<T_DifferentialOperator<DiffOpIdHDivDiv<3>>>();
    // flux_evaluator[VOL] = make_shared<T_DifferentialOperator<DiffOpDivHDivDiv<3>>>();

    evaluator[BND] = make_shared<T_DifferentialOperator<DiffOpIdHDivDivSurface<3>>>();
    auto one = make_shared<ConstantCoefficientFunction>(1);
    // integrator[BND] = make_shared<HDivDivSurfaceMassIntegrator<3>> (one);
    flux_evaluator[BND] =  make_shared<T_DifferentialOperator<DiffOpDivHDivDivSurface<3>>>();
  }

  HDivDivSurfaceSpace :: ~HDivDivSurfaceSpace()
  {
  }


  void HDivDivSurfaceSpace::Update(LocalHeap & lh)
  {
    FESpace::Update(lh);

    nel = ma->GetNSE();
    ndof = 0;

    Array<int> elfacets;
    nfa = ma->GetNEdges();

    first_face_dof.SetSize(nfa + 1);
    Array<int> pnums;

    for (int i = 0; i < nfa; i++)
      {
        first_face_dof[i] = ndof;

        ma->GetEdgePNums(i, pnums);

        switch (pnums.Size())
          {
          case 2: // ET_SEGM
            ndof += order + 1; break;
          case 3: // ET_TRIG
            ndof += (order + 1)*(order + 2) / 2; break;
          case 4: // ET_QUAD
            ndof += (order + 1)*(order + 1); break;
          }
      }
    first_face_dof[nfa] = ndof;

    first_element_dof.SetSize(nel + 1);
    for (int i = 0; i < nel; i++)
      {
        first_element_dof[i] = ndof;

        switch (ma->GetSElType(i))
          {
          case ET_TRIG:
            ndof += 3 * order*(order + 1) / 2;
            break;
          case ET_QUAD:
            ndof += 3 * (order + 1) * (order + 1) - 2;
            break;
          default:
            break;
          }
      }
    first_element_dof[nel] = ndof;

    Array<int> isDirEdge;
    isDirEdge.SetSize(nfa);
    isDirEdge = 0;        

    for (int i = 0; i < nel; i++)
      {
        Ngs_Element ngel = ma->GetSElement (i);

        for(int j=0; j < ngel.edges.Size(); j++)
          isDirEdge[ngel.edges[j]]++;
      }

    if (noncontinuous)
      {
        cout << "Update before discont" << endl;
        ndof = 0;
        Array<int> facets, pnums;
			
        for (int i = 0; i < nel; i++)
          {
            first_element_dof[i] = ndof;
            ma->GetSElEdges(i, facets);

            // add facet dofs
            for (int fa = 0; fa < facets.Size(); fa++)
              {
                ndof += first_face_dof[facets[fa] + 1] - first_face_dof[facets[fa]];
              }

            // add inner dofs
            switch (ma->GetSElType(i))
              {
              case ET_TRIG:
                ndof += 3 * order*(order + 1) / 2;
                break;
              case ET_QUAD:
                ndof += 3 * (order + 1) * (order + 1) - 2;
                break;
              default:
                break;
              }
				
          }
        first_element_dof[nel] = ndof;
        first_face_dof = 0;
        cout << "fed" << first_element_dof << endl;
        cout << "ffd" << first_face_dof << endl;

      }



    //cout << "isDirFaces: " << endl << isDirEdge << endl;
    //cout << "dirichlet_edge before" << endl << dirichlet_edge << endl;
    for(int i = 0; i < isDirEdge.Size(); i++)
      {
        //            if(isDirEdge[i] == 1)
        //                dirichlet_edge[i] = true;
      }
    //cout << "dirichlet_edge after" << endl << dirichlet_edge << endl;
    
  }



  FiniteElement & HDivDivSurfaceSpace::GetFE_old(int elnr, LocalHeap & lh) const
  {
    throw Exception("Volume elements not available for HDivDivSurfaceSpace");
  }

  FiniteElement & HDivDivSurfaceSpace::GetSFE_old(int elnr, LocalHeap & lh) const
  {
    HDivDivFiniteElement<2> * fe = nullptr;

    ArrayMem<int, 4> vnums;
    ma->GetSElVertices(elnr, vnums);


    switch (ma->GetSElType(elnr))
      {
      case ET_TRIG:
        {
          auto * trigfe = new (lh) HDivDivFE<ET_TRIG> (order, false /* plus */);
          trigfe->SetVertexNumbers(vnums);
          trigfe->ComputeNDof();
          fe = trigfe;
          break;
        }
        // case ET_QUAD:
        // fe = new (lh.Alloc(sizeof(HDivSymQuad))) HDivSymQuad();
        // break;
      default:
        cerr << "element type " << int(ma->GetSElType(elnr)) << " not there in hdivsymsurf" << endl;
      }

    ArrayMem<int, 4> ednums;
    Array<INT<2> > order_ed;

    // fe->Noncontinuous() = noncontinuous;

    ma->GetSElEdges(elnr, ednums);

    order_ed.SetSize(ednums.Size());
    for (int j = 0; j < ednums.Size(); j++)
      order_ed[j] = order;

    // fe->SetOrderFacet(order_ed);
    // fe->SetOrderInner(order);
    return *fe;
  }



  void HDivDivSurfaceSpace::GetDofNrs(int elnr, Array<int> & dnums) const
  {
    dnums.SetSize(0);
  }


  void HDivDivSurfaceSpace::GetSDofNrs(int elnr, Array<int> & dnums) const
  {
    dnums.SetSize(0);

    ArrayMem<int, 12> vnums, ednums, fanums;
    int i, j;
    int first, next;

    ma->GetSElEdges(elnr, ednums);

    for (i = 0; i < ednums.Size(); i++)
      {
        first = first_face_dof[ednums[i]];
        next = first_face_dof[ednums[i] + 1];
        for (j = first; j < next; j++)
          dnums.Append(j);
      }

    first = first_element_dof[elnr];
    next = first_element_dof[elnr + 1];
    for (int j = first; j < next; j++)
      dnums.Append(j);
  }

  static RegisterFESpace<HDivDivSurfaceSpace> init("hdivdivsurf");
}
