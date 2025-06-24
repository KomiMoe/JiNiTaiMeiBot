鸡你太美 BOT 自动发车工具

## 蓝奏云下载 
https://wwwi.lanzouo.com/b032d1ec8f
密码: 2g1u

## 使用方法
- 1. 设置出生点为事务所
  2. 切换成第一人称
  3. 打开JiNiTaiMeiBot.exe
  4. 点击Steam聊天框
  5. 点进游戏窗口
  6. 打两年半的篮球

## 配置文件(config.json)

```json
{
	"checkLoopTime" : 1, // 检测间隔时间 (秒)
	"crossAisleTime" : 3000, // 差事层进行"穿过走道"动作持续时间 (毫秒)
	"debug" : true, // 开启调试模式
	"delaySuspendTime" : 5, // 卡单延迟时间 (秒)(在面板消失xx秒后卡单/在落地xx秒后卡单)
	"exitMatchTimeout" : 120, // 等待差事启动落地超时时间 (秒)(防止卡在启动战局中)
	"goOutStairsTime" : 1000, // 差事层楼梯口进行"走出门"动作持续时间 (毫秒)
	"jobTpBotIndex" : -1, // 差传bot的序号，-1: 不指定, 按顺序加入直到成功, 0: 第一个差传(一般为辅助瞄准), 1: 第二个差传(一般为自由瞄准); 
	"joiningPlayerKick" : 120, // 等待正在加入玩家超时重开时间 (秒)
	"matchPanelTimeout" : 300, // 面板无人加入时重开时间 (秒)
	"msgJobStarted" : "开了，请等待下一辆车", // 差事开始时发的消息 (设置为空字符串则不发这条消息)
	"msgJoiningPlayerKick" : "任务中含有卡B，重新启动中", // 有人卡在正在加入超时重开时发的消息 (设置为空字符串则不发这条消息)
	"msgOpenJobPanel" : "德瑞差事已启动，卡好CEO直接来", // 开好面板时发的消息 (设置为空字符串则不发这条消息)
	"msgTeamFull" : "满了", // 满人时发的消息 (设置为空字符串则不发这条消息)
	"msgWaitPlayerTimeout" : "启动任务超时，重新启动中", // 没人加入超时重开时发的消息 (设置为空字符串则不发这条消息)
	"ocrArgs" : "--models=\".\\models\" --det=ch_PP-OCRv4_det_infer.onnx --cls=ch_ppocr_mobile_v2.0_cls_infer.onnx --rec=rec_ch_PP-OCRv4_infer.onnx  --keys=dict_chinese.txt --padding=70 --maxSideLen=1024 --boxScoreThresh=0.5 --boxThresh=0.3 --unClipRatio=1.6 --doAngle=0 --mostAngle=0 --numThread=1", // OCR参数
	"ocrTimeout" : 3, // 等待OCR结果超时时间(秒)
	"pressATimeAisle" : 500, // 差事层进行"穿过走道"动作时 每轮按A的持续时间 (毫秒)
	"pressATimeGoJob" : 480, // 差事层进行"寻找差事黄圈"动作时 每轮按A的持续时间 (毫秒)
	"pressATimeStairs" : 400, // 差事层楼梯口进行"走出门"动作时 每轮按A的持续时间 (毫秒)
	"pressSTimeAisle" : 360, // 差事层进行"穿过走道"动作时 每轮按S的持续时间 (毫秒)
	"pressSTimeGoJob" : 340, // 差事层进行"寻找差事黄圈"动作时 每轮按S的持续时间 (毫秒)
	"pressSTimeStairs" : 400, // 差事层楼梯口进行"走出门"动作时 每轮按S的持续时间 (毫秒)
	"startMatchDelay" : 15, // 开始差事等待延迟 (秒)
	"startOnAllJoined" : true, // 全部玩家已加入时立即开始差事而不等待 (绕过 "startMatchDelay" 时间)
	"suspendAfterMatchStarted" : false, // true: 等待落地后延迟再卡单; false: 面板消失后延迟再卡单
	"suspendGTATime" : 15, // 卡单持续时间 (秒)
	"waitFindJobTimeout" : 15000 // 差事层进行"寻找差事黄圈"动作超时时间 (毫秒)
}
```

## 注意事项
- 自动重启游戏后正常的工作条件：需要故事模式存档为第一人称 且 上线后无需接听富兰克林的电话直接开启终章
