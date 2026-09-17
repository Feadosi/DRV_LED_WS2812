from pathlib import Path
import subprocess, tempfile, re
root=Path(__file__).resolve().parents[1]
for count in (1,2,3,7,12,13):
    with tempfile.TemporaryDirectory() as tmp:
        d=Path(tmp)
        (d/'tim.h').write_text((root/'tests/tim_mock.h').read_text())
        header=(root/'Core/Inc/ws2812.h').read_text()
        (d/'ws2812.h').write_text(re.sub(r'(#define WS2812_LED_COUNT\s+)12U',lambda m:m[1]+str(count)+'U',header))
        exe=d/'test.exe'
        subprocess.run(['gcc','-std=c11','-Wall','-Wextra','-Werror','-I'+str(d),'-I'+str(root/'Core/Src'),str(root/'tests/test_ws2812.c'),'-o',str(exe)],check=True)
        subprocess.run([str(exe)],check=True)
