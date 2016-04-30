# quadratino
A (shitty) Snake clone for GameBoy written in C with gbdk. This should a decent start point to code something with gbdk.

Take a look at [awesome-gbdev](https://github.com/avivace/awesome-gbdev), a curated list of awesome GameBoy Development resources, tools, docs, related projects and open-source ROMs. 

Compile with gbdk

`bin\lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c -o  quadratino.o  quadratino.c
bin\lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -o  quadratino.gb  quadratino.o`

or download the [ready ROM](https://github.com/avivace/quadratino/raw/master/quadratino.gb) (may not be the latest build)

Reccomended emulator: [bgb](http://bgb.bircd.org/)


