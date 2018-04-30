package gui

import (
	"../buffer"
	"go.vktec.org.uk/gopan"
	"go.vktec.org.uk/gopan/vtkcairo"
	"go.vktec.org.uk/vtk"
)

const TextPadding = 5

type TextView struct {
	w vtk.Window
	l gopancairo.CairoLayout
	buf buffer.Buffer
	scroll, scrollStep float64
}

func (ui UI) NewTextView() TextView {
	l := gopancairo.CreateLayout(ui.win.Cairo())
	l.SetWrap(gopan.WordChar)

	fdesc := gopan.FontDescriptionFromString("Helvetica 11")
	l.SetFontDescription(fdesc)

	font := gopancairo.DefaultFontMap().LoadFont(l.Context(), fdesc)
	metrics := font.Metrics()
	asc := metrics.Ascent()
	desc := metrics.Descent()
	lineHeight := (asc + desc) / gopan.Scale

	return TextView{ ui.win, l, ui.ved.Buf, 0.0, 1.5 * float64(lineHeight) }
}

func (t TextView) Draw() {
	// TODO: configurable indentation
	// TODO: elastic tabstops (see http://nickgravgaard.com/elastic-tabstops/)
	t.l.Cr.MoveTo(TextPadding, 0)
	t.l.Show()
}

func (t TextView) Height() int {
	_, h := t.l.PixelSize()
	return h
}

func (t TextView) Resize() {
	t.l.Update()
}

func (t TextView) ScrollBy(amount float64) {
	_, h := t.w.Size()
	scrollMax := float64(h) / 2

	t.scroll += amount * t.scrollStep
	if t.scroll > scrollMax {
		t.scroll = scrollMax
	}

	if t.buf.AtEOF() {
		scrollMin := scrollMax - float64(t.Height())
		if t.scroll < scrollMin {
			t.scroll = scrollMin
		}
	}

	t.damage()
}

func (t TextView) damage() {
}
