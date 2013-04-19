#ifndef __DW_OUTOFFLOWMGR_HH__
#define __DW_OUTOFFLOWMGR_HH__

#include "core.hh"

namespace dw {

class Textblock;

/**
 * \brief Represents additional data for containing blocks.
 */
class OutOfFlowMgr
{
private:
   enum Side { LEFT, RIGHT };

   Textblock *containingBlock;

   // These two values are set by sizeAllocateStart(), and they are
   // accessable also within sizeAllocateEnd(), and also for the
   // containing block, for which allocation and WAS_ALLOCATED is set
   // *after* sizeAllocateEnd(). See the two inline functions below
   // for usage.
   core::Allocation containingBlockAllocation;
   bool containingBlockWasAllocated;

   class Float: public lout::object::Comparable
   {
   private:
      OutOfFlowMgr *oofm;
      
   public:
      class CompareSideSpanningIndex: public lout::object::Comparator
      {
         int compare(Object *o1, Object *o2);
      };

      core::Widget *widget;
      Textblock *generatingBlock;
      int externalIndex;
      int yReq, yReal; // relative to generator, not container
      int index;       /* When GB is not yet allocated: position
                          within TBInfo::leftFloatsGB or
                          TBInfo::rightFloatsGB, respectively. When GB
                          is allocated: position within leftFloatsCB
                          or rightFloatsCB, respectively, even when
                          the floats are still elements of
                          TBInfo::*FloatsGB. */
      int sideSpanningIndex, mark;
      core::Requisition size;
      bool dirty;

      Float (OutOfFlowMgr *oofm, core::Widget *widget,
             Textblock *generatingBlock, int externalIndex);

      void intoStringBuffer(lout::misc::StringBuffer *sb);
      int compareTo(Comparable *other);

      int yForTextblock (Textblock *textblock, int y);
      inline int yForTextblock (Textblock *textblock)
      { return yForTextblock (textblock, yReal); }
      int yForContainer (int y);
      inline int yForContainer () { return yForContainer (yReal); }
      bool covers (Textblock *textblock, int y, int h);
   };

   /**
    * This list is kept sorted.
    *
    * To prevent accessing methods of the base class in an
    * uncontrolled way, the inheritance is private, not public; this
    * means that all methods must be delegated (see iterator(), size()
    * etc. below.)
    *
    * TODO Update comment: still sorted, but ...
    *
    * More: add() and change() may check order again.
    */
   class SortedFloatsVector: private lout::container::typed::Vector<Float>
   {
   public:
      enum Type { GB, CB };

   private:
      OutOfFlowMgr *oofm;
      Side side;
      Type type; // Only used for debugging; may be removed later.
          
   public:
      inline SortedFloatsVector (OutOfFlowMgr *oofm, Side side, Type type) :
         lout::container::typed::Vector<Float> (1, false)
      { this->oofm = oofm; this->side = side; this->type = type; }

      int findFloatIndex (Textblock *lastGB, int lastExtIndex);
      int find (Textblock *textblock, int y, int start, int end);
      int findFirst (Textblock *textblock, int y, int h, Textblock *lastGB,
                     int lastExtIndex);
      int findLastBeforeSideSpanningIndex (int sideSpanningIndex);
      inline void put (Float *vloat)
      { lout::container::typed::Vector<Float>::put (vloat);
        vloat->index = size() - 1; }
      inline void change (Float *vloat) { }

      inline lout::container::typed::Iterator<Float> iterator()
      { return lout::container::typed::Vector<Float>::iterator (); }
      inline int size ()
      { return lout::container::typed::Vector<Float>::size (); }
      inline Float *get (int pos)
      { return lout::container::typed::Vector<Float>::get (pos); }
      inline void clear ()
      { lout::container::typed::Vector<Float>::clear (); }
   };

   class TBInfo: public lout::object::Object
   {
   public:
      bool wasAllocated;
      int xCB, yCB; // relative to the containing block
      int width, height;
      int index; // position within "tbInfos"
      Textblock *textblock; // for debugging; may be removed again

      // These two lists store all floats generated by this textblock,
      // as long as this textblock is not allocates.
      SortedFloatsVector *leftFloatsGB, *rightFloatsGB;

      TBInfo (OutOfFlowMgr *oofm, Textblock *textblock);
      ~TBInfo ();
   };

   // These two lists store all floats, in the order in which they are
   // defined. Only used for iterators.
   lout::container::typed::Vector<Float> *leftFloatsAll, *rightFloatsAll;

   // These two lists store all floats whose generators are already
   // allocated.
   SortedFloatsVector *leftFloatsCB, *rightFloatsCB;

   lout::container::typed::HashTable<lout::object::TypedPointer
                                     <dw::core::Widget>, Float> *floatsByWidget;

   lout::container::typed::Vector<TBInfo> *tbInfos;
   lout::container::typed::HashTable<lout::object::TypedPointer <Textblock>,
                                     TBInfo> *tbInfosByTextblock;

   int lastLeftTBIndex, lastRightTBIndex, leftFloatsMark, rightFloatsMark;

   /**
    * Variant of Widget::wasAllocated(), which can also be used within
    * OOFM::sizeAllocateEnd(), and also for the generating block.
    */
   inline bool wasAllocated (core::Widget *widget) {
      return widget->wasAllocated () ||
         (widget == (core::Widget*)containingBlock &&
          containingBlockWasAllocated); }

   /**
    * Variant of Widget::getAllocation(), which can also be used
    * within OOFM::sizeAllocateEnd(), and also for the generating
    * block.
    */
   inline core::Allocation *getAllocation (core::Widget *widget) {
      return widget == (core::Widget*)containingBlock ?
         &containingBlockAllocation : widget->getAllocation (); }
   
   void moveExternalIndices (SortedFloatsVector *list, int oldStartIndex,
                             int diff);
   Float *findFloatByWidget (core::Widget *widget);

   void moveFromGBToCB (Side side);
   void sizeAllocateFloats (Side side);
   bool isTextblockCoveredByFloats (Textblock *tb, int tbx, int tby,
                                    int tbWidth, int tbHeight, int *floatPos,
                                    core::Widget **vloat);
   bool isTextblockCoveredByFloats (SortedFloatsVector *list, Textblock *tb,
                                    int tbx, int tby, int tbWidth, int tbHeight,
                                    int *floatPos, core::Widget **vloat);

   void draw (SortedFloatsVector *list, core::View *view,
              core::Rectangle *area);
   core::Widget *getWidgetAtPoint (SortedFloatsVector *list, int x, int y,
                                   int level);
   bool collides (Float *vloat, Float *other, int *yReal);
   void checkCoverage (Float *vloat, int oldY);

   void getFloatsLists (Float *vloat, SortedFloatsVector **listSame,
                        SortedFloatsVector **listOpp);

   int getFloatsSize (SortedFloatsVector *list);
   void accumExtremes (SortedFloatsVector *list, int *oofMinWidth,
                       int *oofMaxWidth);
   TBInfo *registerCaller (Textblock *textblock);
   int getBorder (Textblock *textblock, Side side, int y, int h,
                  Textblock *lastGB, int lastExtIndex);
   SortedFloatsVector *getFloatsListForTextblock (Textblock *textblock,
                                                  Side side);
   bool hasFloat (Textblock *textblock, Side side, int y, int h,
                  Textblock *lastGB, int lastExtIndex);

   void ensureFloatSize (Float *vloat);
   int getBorderDiff (Textblock *textblock, Float *vloat, Side side);


   inline static bool isRefFloat (int ref)
   { return ref != -1 && (ref & 1) == 1; }
   inline static bool isRefLeftFloat (int ref)
   { return ref != -1 && (ref & 3) == 1; }
   inline static bool isRefRightFloat (int ref)
   { return ref != -1 && (ref & 3) == 3; }

   inline static int createRefLeftFloat (int index)
   { return (index << 2) | 1; }
   inline static int createRefRightFloat (int index)
   { return (index << 2) | 3; }

   inline static int getFloatIndexFromRef (int ref)
   { return ref == -1 ? ref : (ref >> 2); }

public:
   OutOfFlowMgr (Textblock *containingBlock);
   ~OutOfFlowMgr ();

   void sizeAllocateStart (core::Allocation *containingBlockAllocation);
   void sizeAllocateEnd ();
   void draw (core::View *view, core::Rectangle *area);

   void markSizeChange (int ref);
   void markExtremesChange (int ref);
   core::Widget *getWidgetAtPoint (int x, int y, int level);

   static bool isWidgetOutOfFlow (core::Widget *widget);
   void addWidget (core::Widget *widget, Textblock *generatingBlock,
                   int externalIndex);
   void moveExternalIndices (Textblock *generatingBlock, int oldStartIndex,
                             int diff);

   void tellPosition (core::Widget *widget, int yReq);

   void getSize (int cbWidth, int cbHeight, int *oofWidth, int *oofHeight);
   void getExtremes (int cbMinWidth, int cbMaxWidth, int *oofMinWidth,
                     int *oofMaxWidth);

   int getLeftBorder (Textblock *textblock, int y, int h, Textblock *lastGB,
                      int lastExtIndex);
   int getRightBorder (Textblock *textblock, int y, int h, Textblock *lastGB,
                       int lastExtIndex);

   bool hasFloatLeft (Textblock *textblock, int y, int h, Textblock *lastGB,
                      int lastExtIndex);
   bool hasFloatRight (Textblock *textblock, int y, int h, Textblock *lastGB,
                       int lastExtIndex);

   inline static bool isRefOutOfFlow (int ref)
   { return ref != -1 && (ref & 1) != 0; }
   inline static int createRefNormalFlow (int lineNo) { return lineNo << 1; }
   inline static int getLineNoFromRef (int ref)
   { return ref == -1 ? ref : (ref >> 1); }

   // for iterators
   inline int getNumWidgets () {
      return leftFloatsAll->size() + rightFloatsAll->size(); }
   inline core::Widget *getWidget (int i) {
      return i < leftFloatsAll->size() ? leftFloatsAll->get(i)->widget :
         rightFloatsAll->get(i - leftFloatsAll->size())->widget; }
};

} // namespace dw

#endif // __DW_OUTOFFLOWMGR_HH__
