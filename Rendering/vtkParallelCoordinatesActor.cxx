/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkParallelCoordinatesActor.cxx
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
#include "vtkParallelCoordinatesActor.h"

#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include "vtkViewport.h"

vtkCxxRevisionMacro(vtkParallelCoordinatesActor, "1.22");
vtkStandardNewMacro(vtkParallelCoordinatesActor);

vtkCxxSetObjectMacro(vtkParallelCoordinatesActor,Input,vtkDataObject);
vtkCxxSetObjectMacro(vtkParallelCoordinatesActor,LabelTextProperty,vtkTextProperty);
vtkCxxSetObjectMacro(vtkParallelCoordinatesActor,TitleTextProperty,vtkTextProperty);

//----------------------------------------------------------------------------
// Instantiate object
vtkParallelCoordinatesActor::vtkParallelCoordinatesActor()
{
  this->PositionCoordinate->SetCoordinateSystemToNormalizedViewport();
  this->PositionCoordinate->SetValue(0.1,0.1);
  
  this->Position2Coordinate->SetValue(0.9, 0.8);
  
  this->IndependentVariables = VTK_IV_COLUMN;
  this->N = 0;

  this->Input = NULL;
  this->Axes = NULL; 
  this->Mins = NULL;
  this->Maxs = NULL;
  this->Xs = NULL;

  this->Title = NULL;

  this->TitleMapper = vtkTextMapper::New();

  this->TitleActor = vtkActor2D::New();
  this->TitleActor->SetMapper(this->TitleMapper);
  this->TitleActor->GetPositionCoordinate()->SetCoordinateSystemToViewport();

  this->PlotData = vtkPolyData::New();

  this->PlotMapper = vtkPolyDataMapper2D::New();
  this->PlotMapper->SetInput(this->PlotData);

  this->PlotActor = vtkActor2D::New();
  this->PlotActor->SetMapper(this->PlotMapper);

  this->NumberOfLabels = 2;

  this->LabelTextProperty = vtkTextProperty::New();
  this->LabelTextProperty->SetBold(1);
  this->LabelTextProperty->SetItalic(1);
  this->LabelTextProperty->SetShadow(1);
  this->LabelTextProperty->SetFontFamilyToArial();

  this->TitleTextProperty = vtkTextProperty::New();
  this->TitleTextProperty->ShallowCopy(this->LabelTextProperty);

  this->LabelFormat = new char[8]; 
  sprintf(this->LabelFormat,"%s","%-#6.3g");

  this->LastPosition[0] = 
    this->LastPosition[1] = 
    this->LastPosition2[0] = 
    this->LastPosition2[1] = 0;
}

//----------------------------------------------------------------------------
vtkParallelCoordinatesActor::~vtkParallelCoordinatesActor()
{
  this->TitleMapper->Delete();
  this->TitleMapper = NULL;
  this->TitleActor->Delete();
  this->TitleActor = NULL;
  
  if ( this->Input )
    {
    this->Input->Delete();
    this->Input = NULL;
    }

  this->Initialize();
  
  this->PlotData->Delete();
  this->PlotMapper->Delete();
  this->PlotActor->Delete();

  if (this->Title)
    {
    delete [] this->Title;
    this->Title = NULL;
    }
  
  if (this->LabelFormat) 
    {
    delete [] this->LabelFormat;
    this->LabelFormat = NULL;
    }

  this->SetLabelTextProperty(NULL);
  this->SetTitleTextProperty(NULL);
}

//----------------------------------------------------------------------------
// Free-up axes and related stuff
void vtkParallelCoordinatesActor::Initialize()
{
  if ( this->Axes )
    {
    for (int i=0; i<this->N; i++)
      {
      this->Axes[i]->Delete();
      }
    delete [] this->Axes;
    this->Axes = NULL;
    delete [] this->Mins;
    this->Mins = NULL;
    delete [] this->Maxs;
    this->Maxs = NULL;
    delete [] this->Xs;
    this->Xs = NULL;
    }
  this->N = 0;
}

//----------------------------------------------------------------------------
// Plot scalar data for each input dataset.
int vtkParallelCoordinatesActor::RenderOverlay(vtkViewport *viewport)
{
  int renderedSomething=0;

  // Make sure input is up to date.
  if ( this->Input == NULL || this->N <= 0 )
    {
    vtkErrorMacro(<< "Nothing to plot!");
    return 0;
    }

  if ( this->Title != NULL )
    {
    renderedSomething += this->TitleActor->RenderOverlay(viewport);
    }

  this->PlotActor->SetProperty(this->GetProperty());
  renderedSomething += this->PlotActor->RenderOverlay(viewport);

  for (int i=0; i<this->N; i++)
    {
    renderedSomething += this->Axes[i]->RenderOverlay(viewport);
    }
  
  return renderedSomething;
}

//----------------------------------------------------------------------------
int vtkParallelCoordinatesActor::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int renderedSomething = 0;

  // Initialize

  vtkDebugMacro(<<"Plotting parallel coordinates");
  
  // Make sure input is up to date, and that the data is the correct shape to
  // plot.

  if (!this->Input)
    {
    vtkErrorMacro(<< "Nothing to plot!");
    return renderedSomething;
    }

  // Viewport change may not require rebuild

  if (viewport->GetMTime() > this->BuildTime || 
      (viewport->GetVTKWindow() && 
       viewport->GetVTKWindow()->GetMTime() > this->BuildTime))
    {
    int *lastPosition = 
      this->PositionCoordinate->GetComputedViewportValue(viewport);
    int *lastPosition2 =
      this->Position2Coordinate->GetComputedViewportValue(viewport);
    if (lastPosition[0] != this->LastPosition[0] ||
        lastPosition[1] != this->LastPosition[1] ||
        lastPosition2[0] != this->LastPosition2[0] ||
        lastPosition2[1] != this->LastPosition2[1] )
      {
      this->LastPosition[0] = lastPosition[0];
      this->LastPosition[1] = lastPosition[1];
      this->LastPosition2[0] = lastPosition2[0];
      this->LastPosition2[1] = lastPosition2[1];
      this->Modified();
      }
    }
  
  // Check modified time to see whether we have to rebuild.

  this->Input->Update();

  if (this->GetMTime() > this->BuildTime ||
      this->Input->GetMTime() > this->BuildTime ||
      this->LabelTextProperty->GetMTime() > this->BuildTime ||
      this->TitleTextProperty->GetMTime() > this->BuildTime)
    {
    int *size = viewport->GetSize();
    int stringSize[2];

    vtkDebugMacro(<<"Rebuilding plot");

    // Build axes

    if (!this->PlaceAxes(viewport, size))
      {
      return renderedSomething;
      }

    // Build title

    this->TitleMapper->SetInput(this->Title);

    if (this->TitleTextProperty->GetMTime() > this->BuildTime)
      {
      // Shallow copy here since the justification is changed but we still
      // want to allow actors to share the same text property, and in that case
      // specifically allow the title and label text prop to be the same.
      this->TitleMapper->GetTextProperty()->ShallowCopy(
        this->TitleTextProperty);
      this->TitleMapper->GetTextProperty()->SetJustificationToCentered();
      }

    // We could do some caching here, but hey, that's just the title

    vtkAxisActor2D::SetFontSize(viewport, 
                                this->TitleMapper, 
                                size, 
                                1.0,
                                stringSize);

    this->TitleActor->GetPositionCoordinate()->
      SetValue((this->Xs[0]+this->Xs[this->N-1])/2.0,this->YMax+stringSize[1]/2.0);
    this->TitleActor->SetProperty(this->GetProperty());

    this->BuildTime.Modified();

    } // If need to rebuild the plot

  if ( this->Title != NULL )
    {
    renderedSomething += this->TitleActor->RenderOpaqueGeometry(viewport);
    }

  this->PlotActor->SetProperty(this->GetProperty());
  renderedSomething += this->PlotActor->RenderOpaqueGeometry(viewport);

  for (int i=0; i<this->N; i++)
    {
    renderedSomething += this->Axes[i]->RenderOpaqueGeometry(viewport);
    }

  return renderedSomething;
}

//----------------------------------------------------------------------------
int vtkParallelCoordinatesActor::PlaceAxes(vtkViewport *viewport, int *vtkNotUsed(size))
{
  vtkIdType i, j, id;
  vtkDataObject *input = this->GetInput();
  vtkFieldData *field = input->GetFieldData();
  float v;
  
  this->Initialize();

  if ( ! field )
    {
    return 0;
    }
  
  // Determine the shape of the field
  int numColumns = field->GetNumberOfComponents(); //number of "columns"
  vtkIdType numRows = VTK_LARGE_ID; //figure out number of rows
  vtkIdType numTuples;
  vtkDataArray *array;
  for (i=0; i<field->GetNumberOfArrays(); i++)
    {
    array = field->GetArray(i);
    numTuples = array->GetNumberOfTuples();
    if ( numTuples < numRows )
      {
      numRows = numTuples;
      }
    }

  // Determine the number of independent variables
  if ( this->IndependentVariables == VTK_IV_COLUMN )
    {
    this->N = numColumns;
    }
  else //row
    {
    this->N = numRows;
    }

  if ( this->N <= 0 || this->N >= VTK_LARGE_ID )
    {
    this->N = 0;
    vtkErrorMacro(<<"No field data to plot");
    return 0;
    }
  
  // We need to loop over the field to determine the range of
  // each independent variable.
  this->Mins = new float [this->N];
  this->Maxs = new float [this->N];
  for (i=0; i<this->N; i++)
    {
    this->Mins[i] =  VTK_LARGE_FLOAT;
    this->Maxs[i] = -VTK_LARGE_FLOAT;
    }

  if ( this->IndependentVariables == VTK_IV_COLUMN )
    {
    for (j=0; j<numColumns; j++)
      {
      for (i=0; i<numRows; i++)
        {
        v = field->GetComponent(i,j);
        if ( v < this->Mins[j] )
          {
          this->Mins[j] = v;
          }
        if ( v > this->Maxs[j] )
          {
          this->Maxs[j] = v;
          }
        }
      }
    }
  else //row
    {
    for (j=0; j<numRows; j++)
      {
      for (i=0; i<numColumns; i++)
        {
        v = field->GetComponent(j,i);
        if ( v < this->Mins[j] )
          {
          this->Mins[j] = v;
          }
        if ( v > this->Maxs[j] )
          {
          this->Maxs[j] = v;
          }
        }
      }
    }

  // Allocate space and create axes

  // TODO: this should be optimized, maybe by keeping a list of allocated
  // objects, in order to avoid creation/destruction of axis actors
  // and their underlying text properties (i.e. each time an axis is 
  // created, text properties are created and shallow-assigned a
  // font size which value might be "far" from the target font size).
  
  this->Axes = new vtkAxisActor2D* [this->N];
  for (i=0; i<this->N; i++)
    {
    this->Axes[i] = vtkAxisActor2D::New();
    this->Axes[i]->GetPoint1Coordinate()->SetCoordinateSystemToViewport();
    this->Axes[i]->GetPoint2Coordinate()->SetCoordinateSystemToViewport();
    this->Axes[i]->SetRange(this->Mins[i],this->Maxs[i]);
    this->Axes[i]->AdjustLabelsOff();
    this->Axes[i]->SetNumberOfLabels(this->NumberOfLabels);
    this->Axes[i]->SetLabelFormat(this->LabelFormat);
    this->Axes[i]->SetProperty(this->GetProperty());
    // We do not need shallow copy here since we do not modify any attributes
    // in that class and we know that vtkAxisActor2D use ShallowCopy internally
    // so that the size of the text prop is not affected by the automatic
    // adjustment of its text mapper's size.
    this->Axes[i]->SetLabelTextProperty(this->LabelTextProperty);
    }
  this->Xs = new int [this->N];

  // Get the location of the corners of the box
  int *p1 = this->PositionCoordinate->GetComputedViewportValue(viewport);
  int *p2 = this->Position2Coordinate->GetComputedViewportValue(viewport);

  // Specify the positions for the axes
  this->YMin = p1[1];
  this->YMax = p2[1];
  for (i=0; i<this->N; i++)
    {
    this->Xs[i] = (int) (p1[0] + (float)i/((float)this->N) * (p2[0]-p1[0]));
    this->Axes[i]->GetPoint1Coordinate()->SetValue(this->Xs[i], this->YMin);
    this->Axes[i]->GetPoint2Coordinate()->SetValue(this->Xs[i], this->YMax);
    }

  // Now generate the lines to plot
  this->PlotData->Initialize(); //remove old polydata, if any
  vtkPoints *pts = vtkPoints::New();
  pts->Allocate(numRows*numColumns);
  vtkCellArray *lines = vtkCellArray::New();
  this->PlotData->SetPoints(pts);
  this->PlotData->SetLines(lines);
  
  float x[3]; x[2] = 0.0;
  if ( this->IndependentVariables == VTK_IV_COLUMN )
    {
    lines->Allocate(lines->EstimateSize(numRows,numColumns));
    for (j=0; j<numRows; j++)
      {
      lines->InsertNextCell(numColumns);
      for (i=0; i<numColumns; i++)
        {
        x[0] = this->Xs[i];
        v = field->GetComponent(j,i);
        if ( (this->Maxs[i]-this->Mins[i]) == 0.0 )
          {
          x[1] = 0.5 * (this->YMax - this->YMin);
          }
        else
          {
          x[1] = this->YMin + 
            ((v - this->Mins[i]) / (this->Maxs[i] - this->Mins[i])) *
            (this->YMax - this->YMin);
          }
        id = pts->InsertNextPoint(x);
        lines->InsertCellPoint(id);
        }
      }
    }
  else //row
    {
    lines->Allocate(lines->EstimateSize(numColumns,numRows));
    for (j=0; j<numColumns; j++)
      {
      lines->InsertNextCell(numColumns);
      for (i=0; i<numRows; i++)
        {
        x[0] = this->Xs[i];
        v = field->GetComponent(i,j);
        if ( (this->Maxs[i]-this->Mins[i]) == 0.0 )
          {
          x[1] = 0.5 * (this->YMax - this->YMin);
          }
        else
          {
          x[1] = this->YMin + 
            ((v - this->Mins[i]) / (this->Maxs[i] - this->Mins[i])) *
            (this->YMax - this->YMin);
          }
        id = pts->InsertNextPoint(x);
        lines->InsertCellPoint(id);
        }
      }
    }

  pts->Delete();
  lines->Delete();

  return 1;
}

//----------------------------------------------------------------------------
// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
void vtkParallelCoordinatesActor::ReleaseGraphicsResources(vtkWindow *win)
{
  this->TitleActor->ReleaseGraphicsResources(win);
  for (int i=0; this->Axes && i<this->N; i++)
    {
    this->Axes[i]->ReleaseGraphicsResources(win);
    }
}

//----------------------------------------------------------------------------
void vtkParallelCoordinatesActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if (this->TitleTextProperty)
    {
    os << indent << "Title Text Property:\n";
    this->TitleTextProperty->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Title Text Property: (none)\n";
    }

  if (this->LabelTextProperty)
    {
    os << indent << "Label Text Property:\n";
    this->LabelTextProperty->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Label Text Property: (none)\n";
    }

  os << indent << "Input: " << this->Input << "\n";
  os << indent << "Position2 Coordinate: " 
     << this->Position2Coordinate << "\n";
  this->Position2Coordinate->PrintSelf(os, indent.GetNextIndent());
  
  os << indent << "Title: " << (this->Title ? this->Title : "(none)") << "\n";
  os << indent << "Number Of Independent Variables: " << this->N << "\n";
  os << indent << "Independent Variables: ";
  if ( this->IndependentVariables == VTK_IV_COLUMN )
    {
    os << "Columns\n";
    }
  else
    {
    os << "Rows\n";
    }

  os << indent << "Number Of Labels: " << this->NumberOfLabels << "\n";

  os << indent << "Label Format: " << this->LabelFormat << "\n";
}

//----------------------------------------------------------------------------
// Backward compatibility calls

void vtkParallelCoordinatesActor::SetFontFamily(int val) 
{ 
  this->LabelTextProperty->SetFontFamily(val); 
  this->TitleTextProperty->SetFontFamily(val); 
}

int vtkParallelCoordinatesActor::GetFontFamily()
{ 
  return this->TitleTextProperty->GetFontFamily(); 
}

void vtkParallelCoordinatesActor::SetBold(int val)
{ 
  this->LabelTextProperty->SetBold(val); 
  this->TitleTextProperty->SetBold(val); 
}

int vtkParallelCoordinatesActor::GetBold()
{ 
  return this->TitleTextProperty->GetBold(); 
}

void vtkParallelCoordinatesActor::SetItalic(int val)
{ 
  this->LabelTextProperty->SetItalic(val); 
  this->TitleTextProperty->SetItalic(val); 
}

int vtkParallelCoordinatesActor::GetItalic()
{ 
  return this->TitleTextProperty->GetItalic(); 
}

void vtkParallelCoordinatesActor::SetShadow(int val)
{ 
  this->LabelTextProperty->SetShadow(val); 
  this->TitleTextProperty->SetShadow(val); 
}

int vtkParallelCoordinatesActor::GetShadow()
{ 
  return this->TitleTextProperty->GetShadow(); 
}
