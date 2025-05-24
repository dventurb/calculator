<div align ="center">
    <!--LOGO-->
    <a href="github.com/dventurb/calculator">
    <img src="https://github.com/dventurb/calculator/blob/main/icon/calculator.png" alt="Logo" width="64" height="64">
</a>

<!--PROJECT NAME-->
<h1>Calculator</h1>

<!--DESCRIPTION-->
<p align="center">
    A simple desktop calculator built in C using GTK 4, with a nice graphical user interface.
</p>

<!--BADGES-->
<p>
     <img src="https://img.shields.io/badge/platform-linux-lightgrey" />
     <img src="https://img.shields.io/badge/GTK-4.0-blue" />
</p>

<h3>
    <a href="#installation">Installation</a>
    <span>|</span>
    <a href="#dependencies">Dependencies</a>
</h3>
</div>

<p align="center">
    <img src="https://i.imgur.com/g3HDjgD.gif" alt="screenshot">
</p>


## Installation 

```bash 
git clone https://github.com/dventurb/calculator.git
cd calculator
make
```

## Dependencies
- **GTK 4** (`libgtk-4-dev`)
- **GLib** (`libglib2.0-dev`)
- **tinyexpr** (included in `lib/tinyexpr.c` and `lib/tinyexpr.h`)
- **Build tools**: `gcc`, `make`, `pkg-config`

