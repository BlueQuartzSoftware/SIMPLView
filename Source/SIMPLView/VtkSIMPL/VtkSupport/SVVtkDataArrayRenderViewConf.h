/* ============================================================================
 * Copyright (c) 2017 BlueQuartz Software, LLC
 * All rights reserved.
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
 * Neither the names of any of the BlueQuartz Software contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
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
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#ifndef _svvtkdataarrayrednerviewconf_H_
#define _svvtkdataarrayrednerviewconf_H_

#include <string>


class vtkDataArray;

/**
 * @brief The SVVtkDataArrayRenderViewConf class
 */
class SVVtkDataArrayRenderViewConf
{
  
  public:
    SVVtkDataArrayRenderViewConf();
    virtual ~SVVtkDataArrayRenderViewConf();
    
    using EnumType = int;
    enum class ColorMethod
    {
      Point = 1,
      Cell = 2
    };
    
        
    void setName(const std::string &name);
    std::string getName() const;
    
    void setUniqueId(const int uuid);
    int getUniqueId() const;
    
    void setDataArray(vtkDataArray* dataset);
    vtkDataArray* getDataArray() const;
    
    void setColorMethod(ColorMethod method);
    ColorMethod getColorMethod() const;
    
        
    
  protected:
  
  
  private:
    std::string m_Name;
    int m_UniqueId;
    //bool m_IsSelected = false;
    ColorMethod m_ColorMethod =  ColorMethod::Cell;
    
    vtkDataArray* m_DataArray = nullptr;
    
  
    SVVtkDataArrayRenderViewConf(const SVVtkDataArrayRenderViewConf&) = delete; // Copy Constructor Not Implemented
    void operator=(const SVVtkDataArrayRenderViewConf&) = delete; // Operator '=' Not Implemented
};


#endif /* _svvtkdataarrayrednerviewconf_H_ */
