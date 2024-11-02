## 路线图

### 动画系统：

1. 修改CSpriteRenderer组件使其支持瓦片贴图，支持瓦片索引
2. 动画CAnimation组件，支持简单的序列帧动画，组织一系列AnimationClip，自定义条件变量，自定义状态转换逻辑
3. AnimationClip（一系列索引+持续时间）
4. 资源系统支持动画资源的导入和拖放

### 声音系统

1. 加载声音资源
2. AudioPlayer组件，控制声音播放：循环/单次、音量(随摄像机距离衰减 ？)、播放/暂停/停止、切换声音

### 节点树系统

1. 定义索引节点树去组织UUID组件
2. 物体绘制需要查找父节点并获取绝对变化矩阵
3. 物理系统可能要处理碰撞体的形变，改用多边形来描述碰撞体

### 项目系统

1. 打包成可游玩的游戏，具体操作是修改Sandbox子项目，让它只保留runtime并将framebuffer进行渲染

### 其它

1. 将更多组件操作暴露给C#
2. 将渲染、物理、脚本分开成System
3. 细化ScriptGlue，不要堆在一起

### Bug修复

1. 无法进行抗锯齿处理的bug
2. 快捷键、ImGui的事件blocking逻辑