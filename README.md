## 路线图

### 动画系统：

- [x] 修改CSpriteRenderer组件使其支持瓦片贴图，支持瓦片索引
- [x] 动画CAnimation组件，支持简单的序列帧动画，组织一系列AnimationClip，自定义条件变量，自定义状态转换逻辑
- [x] AnimationClip（一系列索引+持续时间）
- [x] 资源系统支持动画资源的导入和拖放

### 物理系统

- [x] 添加碰撞回调给C#

### 脚本系统

- [x] Entity删除

### 资源系统

- [ ] 图片资源元数据，保存如MIN_FILTER之类的参数
- [ ] 右键新建文件夹，删除文件，文件重命名，文件移动

### 声音系统

- [ ] 加载声音资源
- [ ] AudioPlayer组件，控制声音播放：循环/单次、音量(随摄像机距离衰减 ？)、播放/暂停/停止、切换声音

### UI系统

- [x] 支持绘制固定在屏幕的文字
- [ ] UI层级结构
- [x] 按钮和事件

### 节点树系统

- [x] 定义索引节点树去组织UUID组件
- [x] 物体绘制需要查找父节点并获取绝对变化矩阵
- [ ] 物理系统可能要处理碰撞体的形变，改用多边形来描述碰撞体(?)
- [x] 预制体prefab
- [ ] 预制体二次编辑
- [x] 节点隐藏(active)

### 项目系统

- [x] 一个启动程序页面，显示打开或新建项目
- [x] 新建项目会创建.nsproj & .nsasset，不需要创建premake5.lua & WinGen.bat，这两个在编辑器程序上执行
- [x] 打包成可游玩的游戏，具体操作是修改Sandbox子项目，让它只保留runtime并将framebuffer进行渲染
- [x] 项目基本设置：初始场景选择
- [x] 场景转换

### TileMap & Grid

- [ ] 一个Grid定义网格信息 + 辅助线绘制，子节点多个TileMap层(每个TileMap都需要绑定一个Grid父节点才能生效)
- [ ] WorldToCell(position) & GetCellCenterWorld(position)，应该会用到摄像机属性，应提前整理摄像机和BeginScene代码
- [ ] 鼠标放置和清除cell
- [ ] TileSet属性，设置pivot

后续再考虑：

- [ ] 碰撞体的添加：可能要合并多个共面cell的shape，需要一定的算法



### 其它

- [x] 调整项目结构，不要将文件夹定死，去除Assets文件夹
- [ ] 将更多组件操作暴露给C#
- [x] 将渲染、物理、脚本分开成System
- [ ] 细化ScriptGlue，不要堆在一起

### Bug修复

- [ ] 无法进行抗锯齿处理的bug
- [ ] 快捷键、ImGui的事件blocking逻辑
- [ ] 资源被修改要即时响应，重载
- [x] Save scene as... 初始目录应为项目目录
- [x] 不能提供CTransform的删除选项
- [ ] 物理系统正在运行时才能进行SetLinearVelocity等操作
- [x] 脚本修改带rigbody的实体无法得到正确反馈
- [ ] Log字符串无法显示中文
- [ ] ctrl+D复制实体要：复制子实体，复制父实体id