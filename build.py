import os
import subprocess
import sys
env = os.environ.copy()
keys = list(env.keys())
for k in keys:
    if k.upper() == 'HOME':
        del env[k]
env['HOME'] = 'C:/Users/hyper'
sys.exit(subprocess.run(["cmake", "--build", "build", "--target", "npp_liquid_glass", "--config", "Release"], env=env).returncode)
