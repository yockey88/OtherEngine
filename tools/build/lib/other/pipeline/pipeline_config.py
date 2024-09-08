class PipelineConfig:
  engine_path: str
  project_list = []

  def __init__(self, engine_path: str = ".", project_list = []):
    self.engine_path = engine_path
    self.project_list = project_list

  def get_engine_path(self):
    return self.engine_path
  
  def get_project_list(self):
    return self.project_list
  
  def find_project(self, name: str):
    for proj_name, path in self.project_list:
      if proj_name == name:
        return (proj_name, path)
    return None