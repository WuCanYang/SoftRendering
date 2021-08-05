#pragma once

#define PI 3.1415926f

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024


#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024


#define GENERAL false		//模式三选一
#define SHADOW false
#define WIREFRAME true



#define QUATERNION false

//-----------------------模型简化设置

#define EnableMeshSimplify			true


// 0 - 1 的值， 例如为0.9， 那么留下 90% 的三角形数量或顶点数量
#define RemainPercent				0.1f		


//体积权重
#define VolumeImportance__			1.0f

//是否开启体积保护
#define PreserveVolume__			true


//是否检查骨骼
#define CheckBoneBoundaries__		true


//边界约束权重
#define EdgeWeightValue__			128.0f


//边界加锁
#define LockBoundary__				true


//一条边两个顶点颜色不一样，加锁
#define LockDifferentColorEdge		true