"""Package the retained imagegen atlas without repainting its artwork."""
from pathlib import Path
import subprocess
import sys

from PIL import Image

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
folder = HERE / 'textures/final'
folder.mkdir(parents=True, exist_ok=True)
image = Image.open(HERE / 'textures/raw/mushroom.png').convert('RGB')
image = image.resize((512, 512), Image.Resampling.LANCZOS)
image.save(folder / 'mushroom.png')
image.save(folder / 'mushroom.jpg', quality=96, subsampling=0)
subprocess.run([sys.executable, str(ROOT / 'tools/mu_texture.py'), 'wrap', str(folder / 'mushroom.jpg'),
                '--out', str(folder / 'mushroom.OZJ')], cwd=ROOT, check=True)
result = subprocess.check_output([sys.executable, str(ROOT / 'tools/mu_texture.py'), 'check',
                                  str(folder / 'mushroom.OZJ')], cwd=ROOT, text=True)
(folder / 'loader-check.txt').write_text(result)
