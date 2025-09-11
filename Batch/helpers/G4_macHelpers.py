## Some functions to help build mac files, or build python objects from mac files

# def removeMacFromString(string):
#   return string.rstrip('.mac')

def extract_words_after_string(file_path, string):
    with open(file_path, 'r') as file:
        words = []
        for line in file:
            if string in line:
                words += line.split(string)[1].split()
    
    ## formatting output ##
    words=[convert_to_float_if_possible(w) for w in words]
    # print(words)
    if len(words) == 1:
      return words[0]
    elif len(words) == 2:
      return words[0], words[1]
    elif len(words) == 0: ## find nothing
       return None
    else:
      print(words)
      print("error in", __name__)
      exit(0)


def convert_to_float_if_possible(s):
    if s.isdigit():
        return float(s)
    else:
        return s
