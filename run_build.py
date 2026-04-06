import subprocess
import os

vs_path_cmd = r'"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath'
vs_path = subprocess.check_output(vs_path_cmd, shell=True).decode('utf-8').strip()

vsvars_path = os.path.join(vs_path, 'VC', 'Auxiliary', 'Build', 'vcvars64.bat')

run_cmd = f'"{vsvars_path}" && msbuild Little_Box.vcxproj -v:m -clp:ErrorsOnly;WarningsOnly'

cmd_output = subprocess.run(run_cmd, shell=True, text=True, capture_output=True, cwd=r'e:\Files\Code\Little_Box')

with open(r'e:\Files\Code\Little_Box\dump.txt', 'w', encoding='utf-8') as f:
    f.write("=== STDOUT ===\n")
    f.write(cmd_output.stdout)
    f.write("\n=== STDERR ===\n")
    f.write(cmd_output.stderr)
