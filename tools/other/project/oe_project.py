class Project:
  name: str
  path: str

  def __init__(self, name: str, path: str):
    self.name = name
    self.path = path

  def get_name(self):
    return self.name
  
  def get_path(self):
    return self.path