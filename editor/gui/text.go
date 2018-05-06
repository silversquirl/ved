package gui

import (
	"../buffer"
	"go.vktec.org.uk/gopan"
	"go.vktec.org.uk/gopan/vtkcairo"
	"log"
)

const (
	TextPadding = 5
	ViewStepSize = 1024
)

type TextView struct {
	ui *UI
	l gopancairo.CairoLayout
	buf *buffer.Buffer
	scrollStep float64
}

func (ui *UI) NewTextView() *TextView {
	l := gopancairo.CreateLayout(ui.win.Cairo())
	l.SetWrap(gopan.WordChar)

	l.SetFontDescription(ui.fonts.Regular)

	font := gopancairo.DefaultFontMap().LoadFont(l.Context(), ui.fonts.Regular)
	metrics := font.Metrics()
	asc := metrics.Ascent()
	desc := metrics.Descent()
	lineHeight := (asc + desc) / gopan.Scale

	t := &TextView{ ui, l, &ui.ved.Buf, 1.5 * float64(lineHeight) }
	ui.ved.Buf.DamageCB = func() {
		t.damage()
		t.ui.Redraw()
	}
	return t
}

func (t TextView) Draw() {
	// TODO: configurable indentation
	// TODO: elastic tabstops (see http://nickgravgaard.com/elastic-tabstops/)
	// TODO: draw cursor
	t.l.Cr.MoveTo(TextPadding, 0)
	t.l.Show()
}

func (t TextView) Height() int {
	_, h := t.l.PixelSize()
	return h
}

func (t TextView) Resize() {
	t.l.Update()
	w, _ := t.ui.win.Size()
	t.l.SetWidth((w - TextPadding * 2) * gopan.Scale)
	t.damage()
}

func (t *TextView) damage() {
	target := t.ui.heightTarget() + 10 // + 10 to give it some extra space
	t.l.SetText(t.buf.Text())
	for !t.buf.AtEOF() && t.Height() < target {
		if err := t.buf.ExtendView(ViewStepSize); err != nil {
			log.Println(err)
		}
		t.l.SetText(t.buf.Text())
	}
}
