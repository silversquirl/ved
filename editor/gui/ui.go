package gui

import (
	"../"
	"go.vktec.org.uk/vtk"
	"go.vktec.org.uk/vtk/cairo"
)

type UI struct {
	ved *editor.Editor

	root vtk.Root
	win vtk.Window
	QuitCallback func() bool

	colours ColourScheme
	cr cairo.Cairo
	text TextView
	scrollDelta float64
	status StatusBar
}

func New(ved *editor.Editor) (ui *UI, err error) {
	ui = new(UI)
	ui.ved = ved

	ui.root, err = vtk.New()
	if err != nil {
		return
	}

	ui.win, err = ui.root.NewWindow("ved", 0, 0, 800, 600)

	ui.colours.Foreground = Colour{1, 1, 1, 1}
	ui.colours.Background = Colour{0, 0, 0, .6}
	ui.colours.Line = Colour{.5, .5, .5, 1}
	ui.colours.BarBG = Colour{.1, .1, .1, 1}

	ui.cr = ui.win.Cairo()
	ui.text = ui.NewTextView()
	ui.status = ui.NewStatusBar()

	ui.win.SetEventHandler(vtk.Close, func(_ vtk.Event) { ui.Quit() })
	ui.win.SetEventHandler(vtk.Draw, ui.draw)
	ui.win.SetEventHandler(vtk.KeyPress, func(ev vtk.Event) {
		ui.ved.Modes.Current.HandleKey(ev.(vtk.KeyEvent))
	})
	ui.win.SetEventHandler(vtk.Resize, func(_ vtk.Event) { ui.text.Resize() })
	ui.win.SetEventHandler(vtk.Scroll, ui.scroll)

	return
}

func (ui *UI) Redraw() {
	ui.win.Redraw()
}

func (ui *UI) drawFile(fw float64) {
	// Draw EOF and SOF lines
	ui.colours.Line.SetCairo(ui.cr)
	ui.cr.SetLineWidth(1)
	pad := float64(TextPadding)
	sx := pad
	ex := fw - pad * 2

	ui.cr.MoveTo(sx, 0)
	ui.cr.LineTo(ex, 0)
	ui.cr.Stroke()

	y := float64(ui.text.Height())
	ui.cr.MoveTo(sx, y)
	ui.cr.LineTo(ex, y)
	ui.cr.Stroke()

	// Draw text
	ui.colours.Foreground.SetCairo(ui.cr)
	ui.text.Draw()
}

func (ui *UI) draw(_ vtk.Event) {
	w, h := ui.win.Size()
	fw, fh := float64(w), float64(h)
	// ui.cr = ui.win.Cairo()

	// Window coordinates
	ui.cr.Translate(0, 0)

	// Fill background
	ui.cr.Rectangle(0, 0, fw, fh)
	ui.colours.Background.SetCairo(ui.cr)
	ui.cr.Fill()

	// Draw the file
	ui.cr.PushGroup() // Makes Translate not affect outer group
	ui.cr.Translate(0, ui.scrollDelta)
	ui.drawFile(fw)
	ui.cr.PopGroupToSource()
	ui.cr.Paint()

	// Draw status bar
	ui.status.Draw(fw, fh)
}

func (ui *UI) scroll(ev vtk.Event) {
	scroll := ev.(vtk.ScrollEvent)
	ui.scrollBy(scroll.Amount())
	ui.win.Redraw()
}

func (ui *UI) scrollBy(amount float64) {
	_, h := ui.win.Size()
	scrollMax := float64(h) / 2

	ui.scrollDelta += amount * ui.text.scrollStep
	if ui.scrollDelta > scrollMax {
		ui.scrollDelta = scrollMax
	}

	if ui.text.buf.AtEOF() {
		scrollMin := scrollMax - float64(ui.text.Height())
		if ui.scrollDelta < scrollMin {
			ui.scrollDelta = scrollMin
		}
	}

	ui.text.damage()
}

func (ui UI) heightTarget() int {
	_, h := ui.win.Size()
	return h - int(ui.scrollDelta);
}

func (ui *UI) Quit() {
	if ui.QuitCallback == nil || ui.QuitCallback() {
		ui.win.Close()
	}
}

func (ui *UI) Mainloop() {
	ui.win.Mainloop()
}
