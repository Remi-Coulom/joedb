import os
import ycm_core

dirname = os.path.abspath(os.path.dirname(__file__))
database = ycm_core.CompilationDatabase(dirname)
with open(dirname + "/.ycm_extra_conf_default", "r") as file:
 default_file = dirname + file.read().rstrip()

def Settings(**kwargs):
  filename = kwargs['filename']
  compilation_info = database.GetCompilationInfoForFile(filename)
  if not compilation_info.compiler_flags_:
   compilation_info = database.GetCompilationInfoForFile(default_file)

  return {
    'flags': list(compilation_info.compiler_flags_),
    'include_paths_relative_to_dir': compilation_info.compiler_working_dir_,
    'override_filename': filename
  }
