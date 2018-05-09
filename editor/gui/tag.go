package gui

import (
	"../buffer"
	"go.vktec.org.uk/gopan"
)

func ApplyTag(t buffer.Tag, l gopan.Layout, r buffer.Range) {
	alist := l.Attributes()
	if alist.IsNil() {
		alist = gopan.NewAttrList()
		l.SetAttributes(alist)
	}
	start, end := r.ByteRange(nil).Coords()
	if end < 0 {
		end = gopan.IndexToTextEnd
	}

	if t.Mask & buffer.SetForeground != 0 {
		a := gopan.NewForegroundAttr(t.Foreground)
		a.SetStart(start)
		a.SetEnd(end)
		alist.InsertInvalidate(&a)
	}
	if t.Mask & buffer.SetBackground != 0 {
		a := gopan.NewBackgroundAttr(t.Background)
		a.SetStart(start)
		a.SetEnd(end)
		alist.InsertInvalidate(&a)
	}

	if t.Mask & buffer.SetStyle != 0 {
		if t.Style & buffer.Bold != 0 {
			a := gopan.NewWeightAttr(gopan.Bold)
			a.SetStart(start)
			a.SetEnd(end)
			alist.InsertInvalidate(&a)
		}
		if t.Style & buffer.Light != 0 {
			a := gopan.NewWeightAttr(gopan.Light)
			a.SetStart(start)
			a.SetEnd(end)
			alist.InsertInvalidate(&a)
		}
		if t.Style & buffer.Italic != 0 {
			a := gopan.NewStyleAttr(gopan.Italic)
			a.SetStart(start)
			a.SetEnd(end)
			alist.InsertInvalidate(&a)
		}
		if t.Style & buffer.Underline != 0 {
			a := gopan.NewUnderlineAttr(gopan.Single)
			a.SetStart(start)
			a.SetEnd(end)
			alist.InsertInvalidate(&a)
		}
	}

	if t.Mask & buffer.SetFont != 0 {
		a := gopan.NewFontDescAttr(t.Font)
		a.SetStart(start)
		a.SetEnd(end)
		alist.InsertInvalidate(&a)
	}
}

func tagMerge(t, parent buffer.Tag) buffer.Tag {
	if t.Mask & buffer.SetForeground == 0 && parent.Mask & buffer.SetForeground != 0 {
		t.Mask |= buffer.SetForeground
		t.Foreground = parent.Foreground
	}
	if t.Mask & buffer.SetBackground == 0 && parent.Mask & buffer.SetBackground != 0 {
		t.Mask |= buffer.SetBackground
		t.Background = parent.Background
	}
	if t.Mask & buffer.SetStyle == 0 && parent.Mask & buffer.SetStyle != 0 {
		t.Mask |= buffer.SetStyle
		t.Style = parent.Style
	}
	if t.Mask & buffer.SetFont == 0 && parent.Mask & buffer.SetFont != 0 {
		t.Mask |= buffer.SetFont
		t.Font = parent.Font
	}
	return t
}

func (ui UI) GetTag(key string) buffer.Tag {
	if t, ok := ui.tags[key]; ok {
		return tagMerge(t, ui.tags[""])
	} else {
		return ui.tags[""]
	}
}
