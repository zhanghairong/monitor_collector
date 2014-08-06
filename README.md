系统简介：
    数据采集统计服务。主要提供 action 及 error接口
  
  action: 动作统计 projectid curveid 为平台上申请的，page为自定义字段
    按 projectid curveid page ip 为维度进行统计 5min加和入库
  
  error:  错误上报 projectid code 为平台上申请，
    按 projectid code ip 为维度进行统计上报，同样的错误10s内上报一次。
    

--------------------------------------------------------------------------

数据库： mongodb
数据库结构：
  db： action error 按月生成库  如 monitor_action_2014_08  monitor_error_2014_08
  collection： action_{$projectid}_{YYYY}_{MM}_{DD}, error_{$projectid}_{YYYY}_{MM}_{DD}


--------------------------------------------------------------------------

api:
  url:  /stat_action
  filed      |   type    |  example
  projectid  |  string   | 53904aa2557965ca028b4567
  action[]   |  string   | 53993b0b557965ce4a8b4568|req|10  (curveid|page|num)
  ip         |  string   | 2.2.2.2

  example: http://127.0.0.1:50011/stat_action?projectid=53904aa2557965ca028b4567&action[]=53993b0b557965ce4a8b4568|req|10&action[]=53993b0b557965ce4a8b4568|res_succ|9&action[]=53993b0b557965ce4a8b4568|res_fail|1&ip=2.2.2.2

  url:  /error_report
  filed      |   type    |  example
  projectid  |  string   | 53904aa2557965ca028b4567
  code       |   int     | -10250001
  msg        |  string   |  error message
  ip         |  string   | 2.2.2.2

  example: http://127.0.0.1:50011/error_report?projectid=53904aa2557965ca028b4567&ip=2.2.2.2&code=-10250001&msg=error_message


  
  
