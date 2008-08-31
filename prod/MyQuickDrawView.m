/*
 *  MyQuickDrawView.m
 *  roaringdiff - http://www.biscade.com/tools/diff/

/* 
 * Copyright © 2006-2008 Mitch Haile.
 *
 * This file is part of RoaringDiff.
 *
 * RoaringDiff is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * RoaringDiff is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include "diff.h"
#include "rd_prefs.h"
#include "qd_utils.h"
#import "MyQuickDrawView.h"

#define GUTTER_WIDTH 40
#define LEFT_MARGIN 8

typedef struct {
    rd_file_t *f;       /**< The file */
    int pos_y;          /**< Y-position within the bounds */
    int pos_line;       /**< Which line we're on [local to the screen] */
    int line_h;         /**< Line height */
    Rect bounds;        /**< bounds rect of the drawing surface */
    int middle_x;       /**< x position of the middle of bounds (i.e., left+(right-left/2)) */
    short font_id;      /**< QuickDraw font id */
    short font_size;    /**< Font size */
    qd_diff_view_t *v;  /**< Viewport attributes */
    id qdview;          /**< The QuickDraw view object for this renderer */
} qd_render_t;

static void qd_render_file(qd_render_t *r);
static void qd_render_init(qd_render_t *r, Rect *bounds, rd_file_t *f, MyQuickDrawView *qdview);

/*
static void
qd_dump_diff_view(qd_diff_view_t *v)
{
    if (v == NULL) {
        fprintf(stderr, "    NULL\n");
    } else {
        fprintf(stderr, "    v->num_lines       = %d\n", v->num_lines);
        fprintf(stderr, "    v->selected_line   = %d\n", v->selected_line);
        fprintf(stderr, "    v->effective_line  = %d\n", v->effective_line);
        fprintf(stderr, "    v->effective_start = %d\n", v->effective_start);
    }
}

static void
dump_qd_rect(Rect *r)
{
    if (r == NULL) {
        fprintf(stderr, "NULL!\n");
    } else {
        fprintf(stderr, "top:%d, bottom:%d, left:%d, right:%d }\n", r->top, r->bottom, r->left, r->bottom);
    }
}

static void
dump_nsrect(const char *name, NSRect *r)
{
    assert(name != NULL);
    assert(r    != NULL);
    
    fprintf(stderr, "%s: origin { %f, %f }, size { %f, %f }\n", name, r->origin.x, r->origin.y, r->size.width, r->size.height);
}

static void
qd_dump_render(qd_render_t *r)
{
    fprintf(stderr, "%s START\n", __FUNCTION__);
    if (r == NULL) {
        fprintf(stderr, "  NULL\n");
    } else {
        fprintf(stderr, "  f = %p\n", r->f);
        fprintf(stderr, "  pos_y = %d\n", r->pos_y);
        fprintf(stderr, "  pos_line = %d\n", r->pos_line);
        fprintf(stderr, "  line_h = %d\n", r->line_h);
        fprintf(stderr, "  bounds:");
        dump_qd_rect(&r->bounds);
        fprintf(stderr, "  middle_x = %d\n", r->middle_x);
        fprintf(stderr, "  font_id = %d\n", r->font_id);
        fprintf(stderr, "  font_size = %d\n", r->font_size);
        fprintf(stderr, "  v = %p\n", r->v);
        if (r->v != NULL) {
            qd_dump_diff_view(r->v);
        }
        fprintf(stderr, "   qdview = %p\n", r->qdview);
    }
    fprintf(stderr, "%s DONE\n", __FUNCTION__);
}
*/

@implementation MyQuickDrawView

- (id)initWithFrame:(NSRect)frame {
    [super initWithFrame:frame];

    /* Member variable init */
    m_view.selected_line = -1;

    /* left horz scroller init */
    NSRect leftRect;
    leftRect.origin.x = 0.0f;
    leftRect.origin.y = frame.size.height - [NSScroller scrollerWidth];
    leftRect.size.height = [NSScroller scrollerWidth];
    leftRect.size.width = frame.size.width / 2.0f;
    leftHorzScroller = [[MyDiffHorzScroller alloc] initWithFrame:frame];
    [leftHorzScroller setEnabled:YES];
    [self addSubview:leftHorzScroller];
    [leftHorzScroller setTarget:self];
    [leftHorzScroller setAction:@selector(doHorzScroll:)];
    [leftHorzScroller setFloatValue:0.0 knobProportion:0.25];

    /* right horz scroller init */
    NSRect rightRect;
    rightRect.origin.x = 0.0f;
    rightRect.origin.y = frame.size.height - [NSScroller scrollerWidth];
    rightRect.size.height = [NSScroller scrollerWidth];
    rightRect.size.width = frame.size.width / 2.0f;
    rightHorzScroller = [[MyDiffHorzScroller alloc] initWithFrame:frame];
    [rightHorzScroller setEnabled:YES];
    [self addSubview:rightHorzScroller];
    [rightHorzScroller setTarget:self];
    [rightHorzScroller setAction:@selector(doHorzScroll:)];
    [rightHorzScroller setFloatValue:0.0 knobProportion:0.25];

    m_sel_left = "(none)";
    m_sel_right = "(none)";

    return self;
}

- (void)scrollToLine:(int)lineno
{
    m_view.selected_line = lineno;
    int denominator = g_file.n_effective_lines - m_view.num_lines - 2;
    if (denominator > 0) {
        /*
         * We subtract 3, b/c we've already subtracted 2 (XXX WHY 2?).
         *
         * The 1 extra line is so that we show the previous line, which is important when we're 
         * jumping to an 'add' difference, becuase there is no line number on the add, and it 
         * appears we've jumped into the middle of a diff.  this helps clarify.
         */
        float v = ((float) MAX(0, lineno - 3)) / ((float) denominator);
        [vertScroller setEnabled:YES];
        [vertScroller setFloatValue:v];
    } else {
        [vertScroller setEnabled:NO];
    }
    [self setNeedsDisplay:YES];
    [horzView setNeedsDisplay:YES];
}

- (void)doHorzScroll:(MyDiffHorzScroller *)scroller
{
    // The re-display will update the view based on the scroller's new position
    
    NSScrollerPart hit_part = [scroller hitPart];

    float page_amt = 0.2f; // 20% default
    float line_amt = 0.1f; // 10% default
    float v = [scroller floatValue];
    int   longest_line = 0;
    if (scroller == leftHorzScroller) {
        longest_line = diff_get_longest_line_on_left(&g_file);
    } else {
        longest_line = diff_get_longest_line_on_right(&g_file);
    }
    
    if (longest_line > 0) {
        page_amt = ((float) m_view.num_cols) / ((float) longest_line);
    }
    
    if (m_view.num_cols > 0) {
        line_amt = 1.0f / ((float) m_view.num_cols);
    }
    
    switch (hit_part) {
    case NSScrollerDecrementPage:
        v -= page_amt;
        break;
    case NSScrollerKnob:
        break;
    case NSScrollerIncrementPage:
        v += page_amt;
        break;
    case NSScrollerIncrementLine:
        v += line_amt;
        break;
    case NSScrollerDecrementLine:
        v -= line_amt;
        break;
    case NSScrollerNoPart:
    case NSScrollerKnobSlot:
    default:
        break;
    }
    
    if (v < 0.0) {
        v = 0.0;
    }
    
    if (v > 1.0) {
        v = 1.0;
    }

    [scroller setFloatValue:v];
    
    [self setNeedsDisplay:YES];
}

/**
 * This is the handler for the vertical scroll bar event
 */
- (void)doVertScroll:(id)sender
{
    (void) sender;

    NSScrollerPart hit_part = [vertScroller hitPart];

    float v = [vertScroller floatValue];
    float page_amt = 0.2f; // 20% default
    float line_amt = 0.1f; // 10% default
    
    int scroll_amt = g_file.n_effective_lines - m_view.num_lines;
    if (scroll_amt > 0) {
        page_amt = ((float) (m_view.num_lines - 5)) / ((float) scroll_amt);
    }
    
    if (g_file.n_effective_lines > 0) {
        line_amt = 1.0f / ((float) (g_file.n_effective_lines));
    }
    
    switch (hit_part) {
    case NSScrollerDecrementPage:
        v -= page_amt;
        break;
    case NSScrollerKnob:
        break;
    case NSScrollerIncrementPage:
        v += page_amt;
        break;
    case NSScrollerIncrementLine:
        v += line_amt;
        break;
    case NSScrollerDecrementLine:
        v -= line_amt;
        break;
    case NSScrollerNoPart:
    case NSScrollerKnobSlot:
    default:
        break;
    }
    
    if (v < 0.0) {
        v = 0.0;
    }
    
    if (v > 1.0) {
        v = 1.0;
    }

    [vertScroller setFloatValue:v];
    [self setNeedsDisplay:YES];
    [horzView setNeedsDisplay:YES];
}

- (void)tweakFrame
{
    NSRect windowRect       = [[self superview] frame];
    NSRect horzRect         = [horzView frame];
    NSRect viewRect         = [self frame];
    NSRect vertScrollerRect = [vertScroller frame];

    horzRect.size.height         = 45.0f;
    horzRect.size.width          = windowRect.size.width;
    horzRect.origin.x            = 0.0f;
    horzRect.origin.y            = 0.0f;

    viewRect.size.width          = windowRect.size.width - [NSScroller scrollerWidth];
    viewRect.size.height         = windowRect.size.height - horzRect.size.height;
    viewRect.origin.x            = 0.0f;
    viewRect.origin.y            = horzRect.size.height + 0.0f;

    vertScrollerRect.size.width  = [NSScroller scrollerWidth];
    vertScrollerRect.size.height = viewRect.size.height - [NSScroller scrollerWidth];
    vertScrollerRect.origin.x    = viewRect.size.width;
    vertScrollerRect.origin.y    = horzRect.size.height + [NSScroller scrollerWidth] + 2.0f;
    
    [horzView setFrame:horzRect];
    [self setFrame:viewRect];
    [vertScroller setFrame:vertScrollerRect];

    /*
     * Horiztonal scrollers
     */
    NSRect leftHorzScrollerRect  = [leftHorzScroller frame];
    NSRect rightHorzScrollerRect = [rightHorzScroller frame];

    float view_width = viewRect.size.width;
    float view_x_pos = 0.0f;
    if (rd_prefs_get_show_linenos()) {
        // We need to account for the gutter
        view_width -= GUTTER_WIDTH;
        view_x_pos += GUTTER_WIDTH;
    }
    
    leftHorzScrollerRect.size.width   = (view_width / 2.0f) - 1.0f;
    leftHorzScrollerRect.size.height  = [NSScroller scrollerWidth];
    leftHorzScrollerRect.origin.x     = view_x_pos;
    leftHorzScrollerRect.origin.y     = viewRect.size.height - [NSScroller scrollerWidth];

    rightHorzScrollerRect.size.width  = leftHorzScrollerRect.size.width;
    rightHorzScrollerRect.size.height = [NSScroller scrollerWidth];
    rightHorzScrollerRect.origin.x    = view_x_pos + leftHorzScrollerRect.size.width + 3.0f;
    rightHorzScrollerRect.origin.y    = leftHorzScrollerRect.origin.y;

    [leftHorzScroller  setFrame:leftHorzScrollerRect];
    [rightHorzScroller setFrame:rightHorzScrollerRect];
    
    [leftHorzScroller  setNeedsDisplay:YES];
    [rightHorzScroller setNeedsDisplay:YES];
}

- (qd_diff_view_t*)m_view
{
    return &m_view;
}

/**
 * Render the diff rect
 */
- (void)drawRect:(NSRect)rect
{
    (void) rect;
    // This must be first!
    [self tweakFrame];

    RGBColor gutter_col = { 65535, 0, 0 };
    RGBColor dark_gray  = { 0x6666, 0x6666, 0x6666 };
    Rect qdr;
    [self getQuickDrawBoundsRect:&qdr];
    qdr.bottom -= [NSScroller scrollerWidth] - 2;
    
    qd_render_t r;
    qd_render_init(&r, &qdr, &g_file, self);
    
    // Paint the background
    RGBColor col;
    qd_utils_rgbcolor_from_int(rd_prefs_get_color_background(), &col);
    RGBForeColor(&col);
    PaintRect(&r.bounds);
    
    if (rd_prefs_get_show_linenos()) {
        Rect gutter;
        gutter = qdr;
        gutter.right = GUTTER_WIDTH;
        gutter.bottom += [NSScroller scrollerWidth];
        
        // draw the gutter in a lighter shade of the background
        if (col.red < 52428 && col.green < 52428 && col.blue < 52428) {    // check for overflow
            gutter_col.red   = (unsigned short)(((float)col.red)   * 1.2f);
            gutter_col.green = (unsigned short)(((float)col.green) * 1.2f);
            gutter_col.blue  = (unsigned short)(((float)col.blue)  * 1.2f);
            RGBForeColor(&gutter_col);
        }

        PaintRect(&gutter);

        // paint the dark line between the gutter and the content
        RGBForeColor(&dark_gray);
        gutter.left = gutter.right - 2;
        PaintRect(&gutter);
    }
    
    // Based on the window size and the scroller position, figure out where and how much to render.
    int char_width       = TextWidth(" ", 0, 1);
    r.v->num_cols        = (((int) [leftHorzScroller frame].size.width) / char_width) - 4;  // 4 gives us room for ... and a space.
    r.v->num_lines       = (((int) (qdr.bottom - qdr.top)) + (r.font_size / 2)) / (2 + r.font_size);
    int fudge            = r.v->num_lines - 1;
    r.v->effective_start = (int)(([vertScroller floatValue] * (float) (g_file.n_effective_lines - fudge)) + 0.5f);
    
    // update the vertical scroller knob size 
    float perc = 0.1f; // 10% default
    int scroll_amt = g_file.n_effective_lines;    
    if (scroll_amt > 0) {
        perc = ((float) r.v->num_lines) / ((float) (scroll_amt));
        [vertScroller setFloatValue:[vertScroller floatValue] knobProportion:perc];
        [vertScroller setEnabled:YES];
    } else {
        [vertScroller setEnabled:NO];
    }
    [vertScroller setNeedsDisplay:YES];
    
    // update the horz scroller knobs
    int left_cols = diff_get_longest_line_on_left(&g_file);
    if (left_cols > 0) {
        perc = ((float) r.v->num_cols) / ((float) left_cols);
    } else {
        perc = 0.0f;
    }
    [leftHorzScroller setFloatValue:[leftHorzScroller floatValue] knobProportion:perc];
    [leftHorzScroller setEnabled:(left_cols <= r.v->num_cols ? NO : YES)];
    [leftHorzScroller setNeedsDisplay:YES];
    r.v->left_col_start = (int)([leftHorzScroller floatValue] * (float) (left_cols - r.v->num_cols));
    
    int right_cols = diff_get_longest_line_on_right(&g_file);
    if (right_cols > 0) {
        perc = ((float) r.v->num_cols) / ((float) right_cols);
    } else {
        perc = 0.0f;
    }
    [rightHorzScroller setFloatValue:[rightHorzScroller floatValue] knobProportion:perc];
    [rightHorzScroller setEnabled:(right_cols <= r.v->num_cols ? NO : YES)];
    [rightHorzScroller setNeedsDisplay:YES];
    r.v->right_col_start = (int)([rightHorzScroller floatValue] * (float) (right_cols - r.v->num_cols));
    
    qd_render_file(&r);

    // draw some borders so the diff doesn't appear to be floating in space 
    {
        Rect mr;
        mr.left = r.middle_x - 1;
        mr.right = mr.left + 3;
        mr.top = qdr.top;
        mr.bottom = qdr.bottom;
        
        // draw a vertical barrier between the left and right after drawing the diff lines.
        RGBForeColor(&dark_gray);
        mr.bottom += 2 + [NSScroller scrollerWidth];
        PaintRect(&mr);
        
        // Frame the whole view
        qdr.bottom--;
        qdr.bottom--;
        FrameRect(&qdr);

        // If we have the gutter on, we need to cover up a little bit by the left horz scrollbar
        if (rd_prefs_get_show_linenos()) {
            mr.left = 0;
            mr.top = qdr.bottom - 0;
            mr.bottom += 0;
            mr.right = GUTTER_WIDTH - 2;
            ForeColor(whiteColor);
            PaintRect(&mr);
        }
    }
    
#if 0
    ForeColor(redColor);
    MoveTo(0, mouse_click_y);
    LineTo(qdr.right, mouse_click_y);
    MoveTo(mouse_click_x, 0);
    LineTo(mouse_click_x, qdr.bottom);
#endif
}

- (void)keyDown:(NSEvent *)theEvent
{
    (void) theEvent;
}

- (void)getQuickDrawBoundsRect:(Rect *)r
{
    assert(r != NULL);
    
    Rect qdr;
    qdr.left = qdr.top = 0;
    qdr.right  = (short) [self bounds].size.width;
    qdr.bottom = (short) [self bounds].size.height;

    *r = qdr;
}

- (void)mouseDown:(NSEvent *)theEvent
{
    // convert theEvent's location to the window location
    NSPoint windowLoc = [[self superview] convertPoint:[theEvent locationInWindow] fromView:nil];

    // convert the window location to the local view location
    NSPoint viewLoc = [self convertPoint:windowLoc fromView:[self superview]];
    mouse_click_x = (int) viewLoc.x;
    mouse_click_y = (int) viewLoc.y;
    int y = (int) viewLoc.y;
    
    // convert to a line number.
    Rect qdr;
    [self getQuickDrawBoundsRect:&qdr];
    
    qd_render_t r;
    qd_render_init(&r, &qdr, &g_file, self);
    //qd_dump_render(&r);
    
    int line_where = y / r.line_h;
    m_view.selected_line = r.v->effective_start + line_where;
    
    [self setNeedsDisplay:YES];
    [horzView setNeedsDisplay:YES];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    float deltaVert = [theEvent deltaY];
    float v = [vertScroller floatValue];

    deltaVert /= 100.0;
    deltaVert = -deltaVert;
    v += deltaVert;
    
    if (v < 0.0) {
        v = 0.0;
    }
    
    if (v > 1.0) {
        v = 1.0;
    }

    [vertScroller setFloatValue:v];
    [self setNeedsDisplay:YES];
}

- (int)selectedLine
{
    return m_view.selected_line;
}

- (const char*)selectedLeftLine
{
    return m_sel_left;
}

- (const char*)selectedRightLine
{
    return m_sel_right;
}

- (void)setSelectedLeftLine:(const char *)s
{
    m_sel_left = s;
}

- (void)setSelectedRightLine:(const char *)s
{
    m_sel_right = s;
}

- (BOOL)canJumpNextChange
{
    return diff_get_next_diff_for_effective_line(&g_file, m_view.selected_line) != NULL;
}

- (BOOL)canJumpPrevChange
{
    return diff_get_prev_diff_for_effective_line(&g_file, m_view.selected_line) != NULL;
}

- (void)jumpNextChange
{
    rd_diff_t *d = diff_get_next_diff_for_effective_line(&g_file, m_view.selected_line);
    if (d == NULL) {
        // Oh well
        return;
    }
    
    m_view.selected_line = d->effective_line;
    [self scrollToLine:m_view.selected_line];
}

- (void)jumpPrevChange
{
    rd_diff_t *d = diff_get_prev_diff_for_effective_line(&g_file, m_view.selected_line);
    if (d == NULL) {
        // Oh well
        return;
    }
    
    m_view.selected_line = d->effective_line;
    [self scrollToLine:m_view.selected_line];
}

- (BOOL)canJumpNextLine
{
    return m_view.selected_line < g_file.n_effective_lines;
}

- (BOOL)canJumpPrevLine
{
    return m_view.selected_line >= 0;
}

- (void)jumpNextLine
{
    m_view.selected_line++;
    [self scrollToLine:m_view.selected_line];
}

- (void)jumpPrevLine
{
    m_view.selected_line--;
    if (m_view.selected_line < 0) {
        m_view.selected_line = 0;
    }
    [self scrollToLine:m_view.selected_line];
}

@end

#define LEFT 0x1
#define RIGHT 0x2
#define BOTH (LEFT | RIGHT)
#define IS_LEFT(x) ((x & LEFT) != 0)
#define IS_RIGHT(x) ((x & RIGHT) != 0)

static void
qd_render_background(qd_render_t *r, int where, int is_edit)
{
    assert(r != NULL);

    RGBColor the_color;
    int draw_bg = 1;
    
    if (IS_LEFT(where) && !IS_RIGHT(where)) {
        // It's on the left, but not on the right => line deleted 
        qd_utils_rgbcolor_from_int(rd_prefs_get_color_del(), &the_color);
    } else if (!IS_LEFT(where) && IS_RIGHT(where)) {
        // It's on the right, but not on the left => line added
        qd_utils_rgbcolor_from_int(rd_prefs_get_color_add(), &the_color);
    } else if (is_edit) {
        // It's on both sides && it's an edit 
        qd_utils_rgbcolor_from_int(rd_prefs_get_color_change(), &the_color);
    } else {
        // No difference.
        draw_bg = 0;
    }

    // now draw the background (if needed) or the selected box (if needed)
    Rect lr;
    RGBColor save_color;
    
    GetForeColor(&save_color);
    lr.left = 0;
    lr.top = r->pos_y - r->font_size + 1;
    lr.bottom = lr.top + r->line_h + 1;
    lr.right = r->bounds.right - r->bounds.left;
    if (rd_prefs_get_show_linenos()) {
        lr.left += GUTTER_WIDTH;
    }

    if (draw_bg) {
        RGBForeColor(&the_color);
        PaintRect(&lr);
    //    FrameRect(&lr);
    }
    
    if (r->v->selected_line == r->v->effective_line) {
        lr.bottom--; // XXX: HACK: If the line below us has a background, we'll get overlapped
        ForeColor(whiteColor);
        FrameRect(&lr);
    }
    
    RGBForeColor(&save_color);
}

static void
qd_draw_text(short x, short y, const char *s, int start_at, int slen, int num_cols)
{
    ForeColor(blackColor);
    qd_utils_draw_text(x, y, s, rd_prefs_get_tab_width(), start_at + num_cols, start_at, slen);
}

static void
qd_render_tweak_gutter_if_needed(qd_render_t *r, int *did_tweak)
{
    assert(r != NULL);
    assert(did_tweak != NULL);
    
    if (r->v->selected_line == r->v->effective_line) {
        // Draw the gutter as white-on-black
        Rect lr;
        lr.left = 0;
        lr.top = r->pos_y - r->font_size + 1;
        lr.bottom = lr.top + r->line_h + 0;
        lr.right = GUTTER_WIDTH;
        
        RGBColor save_color;
        GetForeColor(&save_color);

        ForeColor(blackColor);
        PaintRect(&lr);
        
        RGBForeColor(&save_color);
        *did_tweak = 1;
    } else {
        *did_tweak = 0;
    }
}

static void
qd_render_string(qd_render_t *r, const char *s, int where, int is_edit)
{
    assert(r != NULL);

    int left = LEFT_MARGIN;
    int slen = strlen(s);

    // set up
    r->pos_y += r->line_h;
    r->pos_line++;

    // prefix
    if (rd_prefs_get_show_linenos()) {
        // Draw nothing here; we're not putting in a line number
        int gutter_tweak = 0;
        qd_render_tweak_gutter_if_needed(r, &gutter_tweak);
        left = GUTTER_WIDTH + LEFT_MARGIN;
    }
    
    qd_render_background(r, where, is_edit);
    
    // content 
    if (IS_LEFT(where)) {
        qd_draw_text(left, r->pos_y, s, r->v->left_col_start, slen, r->v->num_cols);
        if (r->v->selected_line == r->v->effective_line) {
            [r->qdview setSelectedLeftLine:s];
        }
    } else {
        if (r->v->selected_line == r->v->effective_line) {
            [r->qdview setSelectedLeftLine:NULL];
        }
    }

    if (IS_RIGHT(where)) {
        qd_draw_text(LEFT_MARGIN + r->middle_x, r->pos_y, s, r->v->right_col_start, slen, r->v->num_cols);
        if (r->v->selected_line == r->v->effective_line) {
            [r->qdview setSelectedRightLine:s];
        }
    } else {
        if (r->v->selected_line == r->v->effective_line) {
            [r->qdview setSelectedRightLine:NULL];
        }
    }
}

/**
 * Passed lineno = -1 to not draw a line number.
 */
static void
qd_render_gutter_if_needed(qd_render_t *r, int *left, int lineno)
{
    assert(r != NULL);
    assert(left != NULL);
    
    if (!rd_prefs_get_show_linenos()) {
        return;
    }

    RGBColor save_color;
    GetForeColor(&save_color);
    
    int gutter_tweak = 0;
    qd_render_tweak_gutter_if_needed(r, &gutter_tweak);
    
    if (gutter_tweak) {
        ForeColor(whiteColor);
    } else {
        RGBColor dark_gray = { 0x6666, 0x6666, 0x6666 };
        RGBForeColor(&dark_gray);
    }

    if (lineno >= 0) {
        static char prefix[32];
        snprintf(prefix, sizeof(prefix), "%4d", lineno + 1); //r->v->effective_line); //
        int prefix_len = strlen(prefix);
        MoveTo(8, r->pos_y);
        DrawText(prefix, 0, prefix_len);
    }
    
    *left = GUTTER_WIDTH + LEFT_MARGIN;

    RGBForeColor(&save_color);
}

static void
qd_render_line(qd_render_t *r, int lineno, int where)
{
    assert(r    != NULL);
    assert(r->f != NULL);
    assert(r->f->n_effective_lines > lineno);
    
    // set up
    r->pos_y += r->line_h;
    r->pos_line++;
    
    int left = LEFT_MARGIN;
    qd_render_gutter_if_needed(r, &left, lineno);
    
    const char *s = lineno < 0 ? "" : &r->f->bytes[r->f->newlines[lineno]];
    int slen = diff_get_length_for_orig_line(r->f, lineno);
    
    qd_render_background(r, where, FALSE);
    
    // content
    if (IS_LEFT(where)) {
        qd_draw_text(left, r->pos_y, s, r->v->left_col_start, slen, r->v->num_cols);
        if (r->v->selected_line == r->v->effective_line) {
            [r->qdview setSelectedLeftLine:s];
        }
    } else {
        if (r->v->selected_line == r->v->effective_line) {
            [r->qdview setSelectedLeftLine:NULL];
        }
    }
    
    if (IS_RIGHT(where)) {
        qd_draw_text(LEFT_MARGIN + r->middle_x, r->pos_y, s, r->v->right_col_start, slen, r->v->num_cols);
        if (r->v->selected_line == r->v->effective_line) {
            [r->qdview setSelectedRightLine:s];
        }
    } else {
        if (r->v->selected_line == r->v->effective_line) {
            [r->qdview setSelectedRightLine:NULL];
        }
    }
}

static void
qd_render_none(qd_render_t *r, int lineno)
{
    assert(r != NULL);
    qd_render_line(r, lineno, BOTH);
}

static void
qd_render_add(qd_render_t *r, rd_diff_t *d, int lineno)
{
    assert(r    != NULL);
    assert(r->f != NULL);
    assert(d    != NULL);

    /*
     * We're not changing a line at <lineno>, we're inserting after it.  The line at <lineno>
     * still needs to be drawn!
     */
    r->v->effective_line++;
    if (r->v->effective_line < r->v->effective_start) {
    } else {
        qd_render_none(r, lineno);
    }

    // We're adding to the RIGHT at the given LEFT line position
    int i;
    for (i = 0; i < d->n_changes; i++) {
        r->v->effective_line++;
        if (r->v->effective_line < r->v->effective_start) {
            continue;
        }
        
        rd_line_t *line = &d->changes[i];
        char *s = line->line;
        if (s[0] == '>') {
            s = &s[2];
        }
        
        qd_render_string(r, s, RIGHT, FALSE);
    }
}

static void
qd_render_del(qd_render_t *r, rd_diff_t *d)
{
    assert(r    != NULL);
    assert(r->f != NULL);
    assert(d    != NULL);

    // Deleting lines on the LEFT
    int i;
    for (i = 0; i < d->n_deletes; i++) {
        int line = d->deletes[i];
        r->v->effective_line++;
        if (r->v->effective_line < r->v->effective_start) {
            continue;
        }
        
        qd_render_line(r, line, LEFT);
    }
}

static void
qd_render_edit(qd_render_t *r, rd_diff_t *d, int lineno)
{
    assert(r != NULL);
    assert(d != NULL);
    assert(r->f != NULL);
    assert(r->f->n_effective_lines > lineno);
    assert(d->n_deletes > 0 && d->n_changes > 0);    // otherwise, it's just an add or a del.

    // 1. first we need to figure out how many total lines we're going to draw.
    int i;
    int total_lines = MAX(d->n_deletes, d->n_changes);
    
    for (i = 0; i < total_lines; i++) {
        r->v->effective_line++;
        if (r->v->effective_line < r->v->effective_start) {
            continue;
        }
        
        // 2. Find the next add and del line.  We may run out one before the other
        const char *del_line = NULL;
        const char *add_line = NULL;
        int del_line_len = 0;
        int add_line_len = 0;
        if (i < d->n_deletes) {
            del_line = &r->f->bytes[r->f->newlines[d->deletes[i]]];
            del_line_len = diff_get_length_for_orig_line(r->f, d->deletes[i]);
        }
        
        if (i < d->n_changes) {
            add_line = d->changes[i].line;
            if (add_line[0] == '>') {
                add_line = &add_line[2];
            }
            add_line_len = strlen(add_line);
        }
    
        // 3. Now we know what we're going to draw; so do it!
        int left = LEFT_MARGIN;
        r->pos_y += r->line_h;
        r->pos_line++;
        
        qd_render_background(r, BOTH, TRUE);
        qd_render_gutter_if_needed(r, &left, del_line != NULL ? (lineno + i) : -1);
        
        if (del_line != NULL) {
            qd_draw_text(left, r->pos_y, del_line, r->v->left_col_start, del_line_len, r->v->num_cols);
            if (r->v->selected_line == r->v->effective_line) {
                [r->qdview setSelectedLeftLine:del_line];
            }
        } else {
            if (r->v->selected_line == r->v->effective_line) {
                [r->qdview setSelectedLeftLine:NULL];
            }
        }
        
        if (add_line != NULL) {
            qd_draw_text(LEFT_MARGIN + r->middle_x, r->pos_y, add_line, r->v->right_col_start, add_line_len, r->v->num_cols);
            if (r->v->selected_line == r->v->effective_line) {
                [r->qdview setSelectedRightLine:add_line];
            }
        } else {
            if (r->v->selected_line == r->v->effective_line) {
                [r->qdview setSelectedRightLine:NULL];
            }
        }
    }
}

static void
qd_render_file(qd_render_t *r)
{
    assert(r    != NULL);
    assert(r->f != NULL);
    rd_file_t *f = r->f;
    int left_line = -1;
    r->v->effective_line = 0;
    for ( ; left_line < f->n_newlines && r->v->effective_line < r->v->effective_start + r->v->num_lines ; left_line++) {
        rd_diff_t *d = diff_get_diff_for_line(f, left_line);
        if (d != NULL) {
            switch (d->d_type) {
            case D_NONE:
                qd_render_none(r, left_line);
                break;
            case D_ADD:
                qd_render_add(r, d, left_line);
                break;
            case D_DEL:
                qd_render_del(r, d);
                break;
            case D_EDIT:
                qd_render_edit(r, d, left_line);
                break;
            default:
                break;
            }
            
            assert(d->f1_n_lines > 0);
            left_line += d->f1_n_lines - 1;
        } else {
            r->v->effective_line++;
            if (r->v->effective_line < r->v->effective_start) {
                continue;
            }
            qd_render_line(r, left_line, BOTH);
        }
    }
}

static void
qd_render_init(qd_render_t *r, Rect *bounds, rd_file_t *f, MyQuickDrawView *qdview)
{
    assert(r != NULL);
    assert(bounds != NULL);
    assert(f != NULL);

    bzero(r, sizeof(*r));
    r->v = [qdview m_view];
    r->bounds = *bounds;
    if (rd_prefs_get_show_linenos()) {
        r->middle_x = r->bounds.left + ((r->bounds.right - r->bounds.left + GUTTER_WIDTH) / 2);
    } else {
        r->middle_x = r->bounds.left + ((r->bounds.right - r->bounds.left) / 2);
    }
    r->f = f;
    r->font_size = rd_prefs_get_font_size();
    r->line_h = r->font_size + 2;
    r->qdview = qdview; 
    r->v->selected_line = [r->qdview selectedLine];

    GetFNum("\pCourier", &r->font_id);
    TextFont(r->font_id);    
    TextSize(r->font_size);
}
