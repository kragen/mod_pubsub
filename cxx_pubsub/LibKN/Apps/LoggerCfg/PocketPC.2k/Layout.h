#if !defined(LAYOUT_H)
#define LAYOUT_H

#define lmParent    0

enum TEdge {lmLeft, lmTop, lmRight, lmBottom, lmCenter};

enum TWidthHeight {lmWidth = lmCenter + 1, lmHeight};

enum TMeasurementUnits {lmPixels, lmLayoutUnits};

enum TRelationship {
  lmAsIs      = 0,
  lmPercentOf = 1,
  lmAbove     = 2, lmLeftOf = lmAbove,
  lmBelow     = 3, lmRightOf = lmBelow,
  lmSameAs    = 4,
  lmAbsolute  = 5
};

//
// struct TLayoutConstraint
// ~~~~~~ ~~~~~~~~~~~~~~~~~
// Layout constraints are specified as a relationship between an edge/size
// of one window and an edge/size of one of the window's siblings or parent
//
// It is acceptable for a window to have one of its sizes depend on the
// size of the opposite dimension (e.g. width is twice height)
//
// Distances can be specified in either pixels or layout units
//
// A layout unit is defined by dividing the font "em" quad into 8 vertical
// and 8 horizontal segments. we get the font by self-sending WM_GETFONT
// (we use the system font if WM_GETFONT returns 0)
//
// "lmAbove", "lmBelow", "lmLeftOf", and "lmRightOf" are only used with edge
// constraints. They place the window 1 pixel to the indicated side (i.e.
// adjacent to the other window) and then adjust it by "Margin" (e.g. above
// window "A" by 6)
//
// NOTE: "Margin" is either added to ("lmAbove" and "lmLeftOf") or subtracted
//       from("lmBelow" and "lmRightOf") depending on the relationship
//
// "lmSameAs" can be used with either edges or sizes, and it doesn't offset
// by 1 pixel like the above four relationships did. it also uses "Value"
// (e.g. same width as window "A" plus 10)
//
// NOTE: "Value" is always *added*. use a negative number if you want the
//       effect to be subtractive
//
struct TLayoutConstraint {
  HWND               RelWin;            // relative window, lmParent for parent

  UINT               MyEdge       : 4;  // edge or size (width/height)
  TRelationship      Relationship : 4;
  TMeasurementUnits  Units        : 4;
  UINT               OtherEdge    : 4;  // edge or size (width/height)

  // This union is just for naming convenience
  //
  union {
	int  Margin;   // used for "lmAbove", "lmBelow", "lmLeftOf", "lmRightOf"
	int  Value;    // used for "lmSameAs" and "lmAbsolute"
	int  Percent;  // used for "lmPercentOf"
  };
};

//
// struct TEdgeConstraint
// ~~~~~~ ~~~~~~~~~~~~~~~
// Adds member functions for setting edge constraints
//
struct TEdgeConstraint : public TLayoutConstraint {

  // For setting arbitrary edge constraints. use it like this:
  //   metrics->X.Set(lmLeft, lmRightOf, lmParent, lmLeft, 10);
  //
  void    Set(TEdge edge,      TRelationship rel, HWND otherWin,
			  TEdge otherEdge, int value = 0);

  // These four member functions can be used to position your window with
  // respective to a sibling window. you specify the sibling window and an
  // optional margin between the two windows
  //
  void    LeftOf(HWND  sibling, int margin = 0);
  void    RightOf(HWND sibling, int margin = 0);
  void    Above(HWND   sibling, int margin = 0);
  void    Below(HWND   sibling, int margin = 0);

  // These two work on the same edge, e.g. "SameAs(win, lmLeft)" means
  // that your left edge should be the same as the left edge of "otherWin"
  //
  void    SameAs(HWND    otherWin, TEdge edge);
  void    PercentOf(HWND otherWin, TEdge edge, int percent);

  // Setting an edge to a fixed value
  //
  void    Absolute(TEdge edge, int value);

  // Letting an edge remain as-is
  //
  void    AsIs(TEdge edge);
};

//
// struct TEdgeOrSizeConstraint
// ~~~~~~ ~~~~~~~~~~~~~~~~~~~~~
struct TEdgeOrSizeConstraint : public TEdgeConstraint {

  // Redefine member functions defined by TEdgeConstraint that are hidden by
  // TEdgeOrSizeConstraint because of extra/different params
  //
  void    Absolute(TEdge edge, int value);
  void    PercentOf(HWND otherWin, TEdge edge, int percent);
  void    SameAs(HWND otherWin, TEdge edge);
  void    AsIs(TEdge edge);
  void    AsIs(TWidthHeight edge);
};

struct TEdgeOrWidthConstraint : public TEdgeOrSizeConstraint {

  // Redefine member functions defined by TEdgeOrSizeConstraint that are hidden by
  // TEdgeOrWidthConstraint because of extra/different params
  //
  void    Absolute(TEdge edge, int value);
  void    PercentOf(HWND  otherWin, TEdge edge, int percent);
  void    SameAs(HWND     otherWin, TEdge edge);

  // Setting a width/height to a fixed value
  //
  void    Absolute(int value);

  // Percent of another window's width/height (defaults to being the same
  // dimension but could be the opposite dimension, e.g. make my width 50%
  // of my parent's height)
  //
  void    PercentOf(HWND         otherWin,
					int          percent,
					TWidthHeight otherWidthHeight = lmWidth);

  // Same as another window's width/height (defaults to being the same
  // dimension but could be the opposite dimension, e.g. make my width
  // the same as my height)
  //
  void    SameAs(HWND         otherWin,
				 TWidthHeight otherWidthHeight = lmWidth,
				 int          value = 0);

};

struct TEdgeOrHeightConstraint : public TEdgeOrSizeConstraint {

  // Redefine member functions defined by TEdgeOrSizeConstraint that are hidden by
  // TEdgeOrHeightConstraint because of extra/different params
  //
  void    Absolute(TEdge edge, int value);
  void    PercentOf(HWND  otherWin, TEdge edge, int percent);
  void    SameAs(HWND     otherWin, TEdge edge);

  // Setting a width/height to a fixed value
  //
  void    Absolute(int value);

  // Percent of another window's width/height (defaults to being the same
  // dimension but could be the opposite dimension, e.g. make my width 50%
  // of my parent's height)
  //
  void    PercentOf(HWND         otherWin,
					int          percent,
					TWidthHeight otherWidthHeight = lmHeight);

  // Same as another window's width/height (defaults to being the same
  // dimension but could be the opposite dimension, e.g. make my width
  // the same as my height)
  //
  void    SameAs(HWND         otherWin,
				 TWidthHeight otherWidthHeight = lmHeight,
				 int          value = 0);

};

//
//
//
class TLayoutMetrics {
  public:
	TEdgeConstraint         X;      // Horz1 can be lmLeft, lmCenter, lmRight
	TEdgeConstraint         Y;      // Vert1 can be lmTop, lmCenter, lmBottom
	TEdgeOrWidthConstraint  Width;  // Horz2 can be lmWidth, lmCenter, lmRight
	TEdgeOrHeightConstraint Height; // Vert2 can be lmHeight, lmCenter, lmBottom

	// Defaults each co: RelWin=0, MyEdge=(1st from above), Relationship=AsIs
	//
	TLayoutMetrics();

	void SetMeasurementUnits(TMeasurementUnits units);
};

//
// Private structs used
//
struct TChildMetrics;
struct TConstraint;
struct TVariable;

//
// class TLayoutWindowImp
// ~~~~~ ~~~~~~~~~~~~~
// When specifying the layout metrics for a window, there are several options:
// e.g. in the horizontal direction,
//
//  Two Edge Constraints in X and Width
//   1. left edge and right edge
//   2. center edge and right edge
//   3. left edge and center edge
//
//  Edge Constraint and Size constraint in X and Width
//   4. left edge and size
//   5. right edge and size
//   6. center edge and size
//
// The same holds true in the vertical direction for Y and Height
//
// It is also possible to specify "lmAsIs" in which case we use the windows
// current value
//
// Specifying "lmAbsolute" means that we will use whatever is in data member
// "Value"
//
// We just name the fields "X" and "Width" and "Y" and "Height",
// although its okay to place a right or center edge constraint in the
// "Width" field and its also okay to place a right edge constraint in
// the "X" field (i.e. option #3)
//
// However, it's NOT okay to place a width constraint in the "X" or
// "Height" fields or a height constraint in the "Y" or "Width" fields.
//
class TLayoutWindowImp {
  public:
	// Causes the receiver to size/position its children according to the
	// specified layout metrics
	//
	// If you change the layout metrics for a child window call Layout()
	// to have the changes take effect
	//

	TLayoutWindowImp();
	~TLayoutWindowImp();

	void            SetThisWindow(HWND hwnd);
	HWND            GetThisWindow();

	virtual void    Layout();

	void            SetChildLayoutMetrics(HWND child, TLayoutMetrics& metrics);
	bool            GetChildLayoutMetrics(HWND child, TLayoutMetrics& metrics);
	bool            RemoveChildLayoutMetrics(HWND child);

	// call to change calling Layout()
	//
	void            Resize(UINT sizeType, SIZE size);
	void            RemoveChild(HWND child);

  protected:
	SIZE            ClientSize;

  private:

	enum TWhichConstraint {
	  XConstraint,
	  YConstraint,
	  WidthConstraint,
	  HeightConstraint
	};

  	HWND            ThisHwnd;
	TChildMetrics*  ChildMetrics;
	TConstraint*    Constraints;
	TConstraint*    Plan;
	TVariable*      Variables;
	bool            PlanIsDirty;
	int             NumChildMetrics;
	int             FontHeight;

	TChildMetrics*  GetChildMetrics(HWND child);

	void            AddConstraint(TChildMetrics&     metrics,
								  TLayoutConstraint* c,
								  TWhichConstraint   whichContraint);
	void            BuildConstraints(TChildMetrics& childMetrics);
	void            RemoveConstraints(TChildMetrics& childMetrics);

	void            BuildPlan();
	void            ExecutePlan();
	void            ClearPlan();

	int             LayoutUnitsToPixels(int);
	void            GetFontHeight();
};

class TXLayoutBase
{
};

class TXInvalidLayout : public TXLayoutBase
{
};

class TXLayoutIncomplete : public TXLayoutBase
{
};

//----------------------------------------------------------------------------
// Inline implementations

//
inline void TEdgeConstraint::Set(TEdge edge, TRelationship rel, HWND     otherWin,
								 TEdge otherEdge, int value)
{
  RelWin = otherWin;
  MyEdge = edge;
  Relationship = rel;
  OtherEdge = otherEdge;
  Value = value;
}

//
inline void TEdgeConstraint::LeftOf(HWND     sibling, int margin)
{
  Set(lmRight, lmLeftOf, sibling, lmLeft, margin);
}

//
inline void TEdgeConstraint::RightOf(HWND     sibling, int margin)
{
  Set(lmLeft, lmRightOf, sibling, lmRight, margin);
}

//
inline void TEdgeConstraint::Above(HWND     sibling, int margin)
{
  Set(lmBottom, lmAbove, sibling, lmTop, margin);
}

//
inline void TEdgeConstraint::Below(HWND     sibling, int margin)
{
  Set(lmTop, lmBelow, sibling, lmBottom, margin);
}

//
inline void TEdgeConstraint::SameAs(HWND     otherWin, TEdge edge)
{
  Set(edge, lmSameAs, otherWin, edge, 0);
}

//
inline void TEdgeConstraint::PercentOf(HWND     otherWin, TEdge edge, int percent)
{
  Set(edge, lmPercentOf, otherWin, edge, percent);
}

//
inline void TEdgeConstraint::Absolute(TEdge edge, int value)
{
  MyEdge = edge;
  Value = value;
  Relationship = lmAbsolute;
}

//
inline void TEdgeConstraint::AsIs(TEdge edge)
{
  MyEdge = edge;
  Relationship = lmAsIs;
}

//---

//
inline void TEdgeOrSizeConstraint::AsIs(TEdge edge)
{
  TEdgeConstraint::AsIs(edge);
}

//
inline void TEdgeOrSizeConstraint::AsIs(TWidthHeight edge)
{
  TEdgeConstraint::AsIs(TEdge(edge));
}

//---

//
inline void TEdgeOrWidthConstraint::Absolute(TEdge edge, int value)
{
  TEdgeConstraint::Absolute(edge, value);
}

//
inline void TEdgeOrWidthConstraint::PercentOf(HWND     otherWin, TEdge edge, int percent)
{
  TEdgeConstraint::PercentOf(otherWin, edge, percent);
}

//
inline void TEdgeOrWidthConstraint::SameAs(HWND     otherWin, TEdge edge)
{
  TEdgeConstraint::SameAs(otherWin, edge);
}

//
inline void TEdgeOrWidthConstraint::Absolute(int value)
{
  MyEdge = lmWidth;
  Value = value;
  Relationship = lmAbsolute;
}

//
inline void TEdgeOrWidthConstraint::PercentOf(HWND     otherWin,
  int percent, TWidthHeight otherWidthHeight)
{
  RelWin = otherWin;
  MyEdge = lmWidth;
  Relationship = lmPercentOf;
  OtherEdge = otherWidthHeight;
  Percent = percent;
}

//
inline void TEdgeOrWidthConstraint::SameAs(HWND     otherWin,
  TWidthHeight otherWidthHeight, int value)
{
  RelWin = otherWin;
  MyEdge = lmWidth;
  Relationship = lmSameAs;
  OtherEdge = otherWidthHeight;
  Value = value;
}

//---

//
inline void TEdgeOrHeightConstraint::Absolute(TEdge edge, int value)
{
  TEdgeConstraint::Absolute(edge, value);
}

//
inline void TEdgeOrHeightConstraint::PercentOf(HWND     otherWin, TEdge edge, int percent)
{
  TEdgeConstraint::PercentOf(otherWin, edge, percent);
}

//
inline void TEdgeOrHeightConstraint::SameAs(HWND     otherWin, TEdge edge)
{
  TEdgeConstraint::SameAs(otherWin, edge);
}

//
inline void TEdgeOrHeightConstraint::Absolute(int value)
{
  MyEdge = lmHeight;
  Value = value;
  Relationship = lmAbsolute;
}

//
inline void TEdgeOrHeightConstraint::PercentOf(HWND     otherWin,
  int percent, TWidthHeight otherWidthHeight)
{
  RelWin = otherWin;
  MyEdge = lmHeight;
  Relationship = lmPercentOf;
  OtherEdge = otherWidthHeight;
  Percent = percent;
}

//
inline void TEdgeOrHeightConstraint::SameAs(HWND     otherWin,
  TWidthHeight otherWidthHeight, int value)
{
  RelWin = otherWin;
  MyEdge = lmHeight;
  Relationship = lmSameAs;
  OtherEdge = otherWidthHeight;
  Value = value;
}

inline void TLayoutWindowImp::SetThisWindow(HWND hwnd)
{
	ThisHwnd = hwnd;
}

inline HWND TLayoutWindowImp::GetThisWindow()
{
	return ThisHwnd;
}

#endif // LAYOUT_H
