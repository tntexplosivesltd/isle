def get_file_in_script_dir(fn):
  return os.path.join(os.path.dirname(os.path.abspath(sys.argv[0])), fn)

def print_diff(udiff, plain=False):
  for line in udiff:
    color = ''
    if line.startswith('++') or line.startswith('@@') or line.startswith('--'):
      # Skip unneeded parts of the diff for the brief view
      continue
    # Work out color if we are printing color
    if not plain:
      if line.startswith('+'):
        color = colorama.Fore.GREEN
      elif line.startswith('-'):
        color = colorama.Fore.RED
    print(color + line)
    # Reset color if we're printing in color
    if not plain:
      print(colorama.Style.RESET_ALL, end='')
