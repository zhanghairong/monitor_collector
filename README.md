ϵͳ��飺
    ���ݲɼ�ͳ�Ʒ�����Ҫ�ṩ action �� error�ӿ�
  
  action: ����ͳ�� projectid curveid Ϊƽ̨������ģ�pageΪ�Զ����ֶ�
    �� projectid curveid page ip Ϊά�Ƚ���ͳ�� 5min�Ӻ����
  
  error:  �����ϱ� projectid code Ϊƽ̨�����룬
    �� projectid code ip Ϊά�Ƚ���ͳ���ϱ���ͬ���Ĵ���10s���ϱ�һ�Ρ�
    

--------------------------------------------------------------------------

���ݿ⣺ mongodb
���ݿ�ṹ��
  db�� action error �������ɿ�  �� monitor_action_2014_08  monitor_error_2014_08
  collection�� action_{$projectid}_{YYYY}_{MM}_{DD}, error_{$projectid}_{YYYY}_{MM}_{DD}


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


  
  
