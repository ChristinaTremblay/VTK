/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkScalarBarActor.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkScalarBarActor - Create a scalar bar with labels
// .SECTION Description
// vtkScalarBarActor creates a scalar bar with annotation text. A scalar
// bar is a legend that indicates to the viewer the correspondence between
// color value and data value. The legend consists of a rectangular bar 
// made of rectangular pieces each colored a constant value. Since 
// vtkScalarBarActor is a subclass of vtkActor2D, it is drawn in the image 
// plane (i.e., in the renderer's viewport) on top of the 3D graphics window.
//
// To use vtkScalarBarActor you must associate a vtkScalarsToColors (or
// subclass) with it. The lookup table defines the colors and the
// range of scalar values used to map scalar data.  Typically, the
// number of colors shown in the scalar bar is not equal to the number
// of colors in the lookup table, in which case sampling of
// the lookup table is performed. 
//
// Other optional capabilities include specifying the fraction of the
// viewport size (both x and y directions) which will control the size
// of the scalar bar and the number of annotation labels. The actual position
// of the scalar bar on the screen is controlled by using the
// vtkActor2D::SetPosition() method (by default the scalar bar is
// centered in the viewport).  Other features include the ability to
// orient the scalar bar horizontally of vertically and controlling
// the format (printf style) with which to print the labels on the
// scalar bar. Also, the vtkScalarBarActor's property is applied to
// the scalar bar and annotation (including layer, and
// compositing operator).
//
// Set the text property/attributes of the title and the labels through the 
// vtkTextProperty objects associated to this actor.
//
// .SECTION See Also
// vtkActor2D vtkTextProperty vtkTextMapper vtkPolyDataMapper2D

#ifndef __vtkScalarBarActor_h
#define __vtkScalarBarActor_h

#include "vtkActor2D.h"

class vtkPolyData;
class vtkPolyDataMapper2D;
class vtkScalarsToColors;
class vtkTextMapper;
class vtkTextProperty;

#define VTK_ORIENT_HORIZONTAL 0
#define VTK_ORIENT_VERTICAL 1

class VTK_RENDERING_EXPORT vtkScalarBarActor : public vtkActor2D
{
public:
  vtkTypeRevisionMacro(vtkScalarBarActor,vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate object with 64 maximum colors; 5 labels; %%-#6.3g label
  // format, no title, and vertical orientation. The initial scalar bar
  // size is (0.05 x 0.8) of the viewport size.
  static vtkScalarBarActor *New();

  // Description:
  // Draw the scalar bar and annotation text to the screen.
  int RenderOpaqueGeometry(vtkViewport* viewport);
  int RenderTranslucentGeometry(vtkViewport*) { return 0; };
  int RenderOverlay(vtkViewport* viewport);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  virtual void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Set/Get the vtkLookupTable to use. The lookup table specifies the number
  // of colors to use in the table (if not overridden), as well as the scalar
  // range.
  virtual void SetLookupTable(vtkScalarsToColors*);
  vtkGetObjectMacro(LookupTable,vtkScalarsToColors);

  // Description:
  // Set/Get the maximum number of scalar bar segments to show. This may
  // differ from the number of colors in the lookup table, in which case
  // the colors are samples from the lookup table.
  vtkSetClampMacro(MaximumNumberOfColors, int, 2, VTK_LARGE_INTEGER);
  vtkGetMacro(MaximumNumberOfColors, int);
  
  // Description:
  // Set/Get the number of annotation labels to show.
  vtkSetClampMacro(NumberOfLabels, int, 0, 64);
  vtkGetMacro(NumberOfLabels, int);
  
  // Description:
  // Control the orientation of the scalar bar.
  vtkSetClampMacro(Orientation,int,VTK_ORIENT_HORIZONTAL, VTK_ORIENT_VERTICAL);
  vtkGetMacro(Orientation, int);
  void SetOrientationToHorizontal()
       {this->SetOrientation(VTK_ORIENT_HORIZONTAL);};
  void SetOrientationToVertical() {this->SetOrientation(VTK_ORIENT_VERTICAL);};

  // Description:
  // Set/Get the title text property.
  virtual void SetTitleTextProperty(vtkTextProperty *p);
  vtkGetObjectMacro(TitleTextProperty,vtkTextProperty);
  
  // Description:
  // Set/Get the labels text property.
  virtual void SetLabelTextProperty(vtkTextProperty *p);
  vtkGetObjectMacro(LabelTextProperty,vtkTextProperty);
    
  // Description:
  // Set/Get the font family. Three font types are allowed: Arial (VTK_ARIAL),
  // Courier (VTK_COURIER), and Times (VTK_TIMES).
  // Warning: these functions remain for backward compatibility. Use the
  // vtkTextProperty through the (Set/Get)(Title/Label)TextProperty() methods.
  virtual void SetFontFamily(int val);
  virtual int GetFontFamily();
  void SetFontFamilyToArial()   { this->SetFontFamily(VTK_ARIAL);  };
  void SetFontFamilyToCourier() { this->SetFontFamily(VTK_COURIER);};
  void SetFontFamilyToTimes()   { this->SetFontFamily(VTK_TIMES);  };

  // Description:
  // Enable/disable text bolding.
  // Warning: these functions remain for backward compatibility. Use the
  // vtkTextProperty through the (Set/Get)(Title/Label)TextProperty() methods.
  virtual void SetBold(int val);
  virtual int GetBold();
  vtkBooleanMacro(Bold, int);

  // Description:
  // Enable/disable text italic.
  // Warning: these functions remain for backward compatibility. Use the
  // vtkTextProperty through the (Set/Get)(Title/Label)TextProperty() methods.
  virtual void SetItalic(int val);
  virtual int GetItalic();
  vtkBooleanMacro(Italic, int);

  // Description:
  // Enable/disable text shadows.
  // Warning: these functions remain for backward compatibility. Use the
  // vtkTextProperty through the (Set/Get)(Title/Label)TextProperty() methods.
  virtual void SetShadow(int val);
  virtual int GetShadow();
  vtkBooleanMacro(Shadow, int);
  
  // Description:
  // Set/Get the format with which to print the labels on the scalar
  // bar.
  vtkSetStringMacro(LabelFormat);
  vtkGetStringMacro(LabelFormat);

  // Description:
  // Set/Get the title of the scalar bar actor,
  vtkSetStringMacro(Title);
  vtkGetStringMacro(Title);

  // Description:
  // Shallow copy of a scalar bar actor. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop);

protected:
  vtkScalarBarActor();
  ~vtkScalarBarActor();

  vtkScalarsToColors *LookupTable;
  vtkTextProperty *TitleTextProperty;
  vtkTextProperty *LabelTextProperty;

  int   MaximumNumberOfColors;
  int   NumberOfLabels;
  int   NumberOfLabelsBuilt;
  int   Orientation;
  char  *Title;
  char  *LabelFormat;

  vtkTextMapper **TextMappers;
  virtual void AllocateAndSizeLabels(int *labelSize, int *size,
                                     vtkViewport *viewport, float *range);

private:
  vtkTextMapper *TitleMapper;
  vtkActor2D    *TitleActor;

  vtkActor2D    **TextActors;

  vtkPolyData         *ScalarBar;
  vtkPolyDataMapper2D *ScalarBarMapper;
  vtkActor2D          *ScalarBarActor;

  vtkTimeStamp  BuildTime;
  int LastSize[2];
  int LastOrigin[2];

  void SizeTitle(int *titleSize, int *size, vtkViewport *viewport);

private:
  vtkScalarBarActor(const vtkScalarBarActor&);  // Not implemented.
  void operator=(const vtkScalarBarActor&);  // Not implemented.
};


#endif

