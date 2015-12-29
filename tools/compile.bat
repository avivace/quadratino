rem gbdk macro for Sublime Text build system
color 0B
set str=%1
set str2=%str:~1,-3%
del %str2%.gb
bin\lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c -o  %str2%.o  %str2%.c
bin\lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -o  %str2%.gb  %str2%.o
del *.o
taskkill /f /im bgb.exe
start C:\Users\Antuz\gdrive\gbdev\gbdk\bgb.exe %str2%.gb
