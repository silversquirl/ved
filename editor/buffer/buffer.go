package buffer

import (
	"io"
	"os"
	"runtime"
)

type Buffer struct {
	r interface {
		io.ReaderAt
		io.Closer
	}
	buf      []byte
	end      int
	eof      bool
	DamageCB func()
}

func New(filename string) (buf Buffer, err error) {
	buf = Buffer{}
	buf.r, err = os.OpenFile(filename, os.O_RDWR|os.O_CREATE, 0666)
	runtime.SetFinalizer(&buf, func(buf *Buffer) { buf.r.Close() })
	return
}

func (buf Buffer) AtEOF() bool {
	return buf.eof
}

func (buf *Buffer) ExtendView(step int) error {
	off := len(buf.buf)
	tmp := make([]byte, step)
	n, err := buf.r.ReadAt(tmp, int64(off))
	if err == io.EOF {
		buf.eof = true
	} else if err != nil {
		return err
	}
	buf.buf = append(buf.buf, tmp[:n]...)
	buf.end += n
	return nil
}

func (buf Buffer) Text() string {
	return string(buf.buf)
}

// TODO: editing functions. Make sure they call DamageCB
