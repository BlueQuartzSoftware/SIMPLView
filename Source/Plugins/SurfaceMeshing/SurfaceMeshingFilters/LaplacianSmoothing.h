/* ============================================================================
* Copyright (c) 2009-2015 BlueQuartz Software, LLC
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


#ifndef _LaplacianSmoothing_H_
#define _LaplacianSmoothing_H_

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/AbstractFilter.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"

#include "SurfaceMeshing/SurfaceMeshingFilters/SurfaceMeshFilter.h"

#define OUTPUT_DEBUG_VTK_FILES 1

/**
 * @brief The LaplacianSmoothing class. See [Filter documentation](@ref laplaciansmoothing) for details.
 */
class LaplacianSmoothing : public SurfaceMeshFilter
{
    Q_OBJECT /* Need this for Qt's signals and slots mechanism to work */
  public:
    DREAM3D_SHARED_POINTERS(LaplacianSmoothing)
    DREAM3D_STATIC_NEW_MACRO(LaplacianSmoothing)
    DREAM3D_TYPE_MACRO_SUPER(LaplacianSmoothing, SurfaceMeshFilter)

    virtual ~LaplacianSmoothing();

    DREAM3D_FILTER_PARAMETER(QString, SurfaceDataContainerName)
    Q_PROPERTY(QString SurfaceDataContainerName READ getSurfaceDataContainerName WRITE setSurfaceDataContainerName)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, SurfaceMeshNodeTypeArrayPath)
    Q_PROPERTY(DataArrayPath SurfaceMeshNodeTypeArrayPath READ getSurfaceMeshNodeTypeArrayPath WRITE setSurfaceMeshNodeTypeArrayPath)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, SurfaceMeshFaceLabelsArrayPath)
    Q_PROPERTY(DataArrayPath SurfaceMeshFaceLabelsArrayPath READ getSurfaceMeshFaceLabelsArrayPath WRITE setSurfaceMeshFaceLabelsArrayPath)

    DREAM3D_FILTER_PARAMETER(int, IterationSteps)
    Q_PROPERTY(int IterationSteps READ getIterationSteps WRITE setIterationSteps)

    DREAM3D_FILTER_PARAMETER(float, Lambda)
    Q_PROPERTY(float Lambda READ getLambda WRITE setLambda)

    DREAM3D_FILTER_PARAMETER(float, SurfacePointLambda)
    Q_PROPERTY(float SurfacePointLambda READ getSurfacePointLambda WRITE setSurfacePointLambda)

    DREAM3D_FILTER_PARAMETER(float, TripleLineLambda)
    Q_PROPERTY(float TripleLineLambda READ getTripleLineLambda WRITE setTripleLineLambda)

    DREAM3D_FILTER_PARAMETER(float, QuadPointLambda)
    Q_PROPERTY(float QuadPointLambda READ getQuadPointLambda WRITE setQuadPointLambda)

    DREAM3D_FILTER_PARAMETER(float, SurfaceTripleLineLambda)
    Q_PROPERTY(float SurfaceTripleLineLambda READ getSurfaceTripleLineLambda WRITE setSurfaceTripleLineLambda)

    DREAM3D_FILTER_PARAMETER(float, SurfaceQuadPointLambda)
    Q_PROPERTY(float SurfaceQuadPointLambda READ getSurfaceQuadPointLambda WRITE setSurfaceQuadPointLambda)


    /* This class is designed to be subclassed so that thoes subclasses can add
     * more functionality such as constrained surface nodes or Triple Lines. We use
     * this array to assign each vertex a specific Lambda value. Subclasses can set
     * this array then simply call the 'smooth' protected method to actually run the
     * smoothing iterations
     */
    DREAM3D_VIRTUAL_INSTANCE_PROPERTY(DataArray<float>::Pointer, LambdaArray)

    /**
     * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
     */
    virtual const QString getCompiledLibraryName();

    /**
     * @brief newFilterInstance Reimplemented from @see AbstractFilter class
     */
    virtual AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters);

    /**
     * @brief getGroupName Reimplemented from @see AbstractFilter class
     */
    virtual const QString getGroupName();

    /**
     * @brief getSubGroupName Reimplemented from @see AbstractFilter class
     */
    virtual const QString getSubGroupName();

    /**
     * @brief getHumanLabel Reimplemented from @see AbstractFilter class
     */
    virtual const QString getHumanLabel();

    /**
     * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
     */
    virtual void setupFilterParameters();

    /**
     * @brief writeFilterParameters Reimplemented from @see AbstractFilter class
     */
    virtual int writeFilterParameters(AbstractFilterParametersWriter* writer, int index);

    /**
     * @brief readFilterParameters Reimplemented from @see AbstractFilter class
     */
    virtual void readFilterParameters(AbstractFilterParametersReader* reader, int index);

    /**
     * @brief execute Reimplemented from @see AbstractFilter class
     */
    virtual void execute();

    /**
    * @brief preflight Reimplemented from @see AbstractFilter class
    */
    virtual void preflight();

  protected:
    LaplacianSmoothing();

    /**
     * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
     */
    void dataCheck();

    /**
     * @brief generateLambdaArray Generates the Lambda array that will be use during the smoothing
     * @return Integer error code
     */
    virtual int32_t generateLambdaArray();

    /**
     * @brief edgeBasedSmoothing Version of the smoothing algorithm uses Edge->Vertex connectivity information for its algorithm
     * @return Integer error code
     */
    virtual int32_t edgeBasedSmoothing();

  private:
    DEFINE_DATAARRAY_VARIABLE(int8_t, SurfaceMeshNodeType)
    DEFINE_DATAARRAY_VARIABLE(int32_t, SurfaceMeshFaceLabels)

    LaplacianSmoothing(const LaplacianSmoothing&); // Copy Constructor Not Implemented
    void operator=(const LaplacianSmoothing&); // Operator '=' Not Implemented
};

#endif /* _LaplacianSmoothing_H_ */
