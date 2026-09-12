# protobela
A sandpit for developing Bela applications on a desktop machine.

The supplied Makefile builds a static archive containing a main program which
emulates the Bela runtime on a desktop Linux machine. It also builds two small
demonstration programs:

 * `beep` - a minimum application which goes beep
 * `sampleplayer` - an application which loops three samples which can be
    paused and resumed individually using keyboard streams (simulating Bela
    digital I/O).

## Building
On Debian Linux install `librtaudio-dev` and `libsndfile1-dev`. Then just run make.

## Running
`./beep` or `./sampleplayer`

### Command-line Options
 * `--keys=...` sets keys to be regarded as digital inputs.
    Pressing the key toggles the virtual input.
    Input levels are displayed on the terminal.
    Default is `asdf`.
 * `--quit=<char>` sets the key which closes down the
    emulator and quits. Default is ``q``
