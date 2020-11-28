#include"Boss/Mod/OnchainFeeMonitor.hpp"
#include"Boss/Mod/Rpc.hpp"
#include"Boss/Mod/Waiter.hpp"
#include"Boss/Msg/DbResource.hpp"
#include"Boss/Msg/Init.hpp"
#include"Boss/Msg/OnchainFee.hpp"
#include"Boss/Msg/ProvideStatus.hpp"
#include"Boss/Msg/SolicitStatus.hpp"
#include"Boss/Msg/Timer10Minutes.hpp"
#include"Boss/concurrent.hpp"
#include"Boss/log.hpp"
#include"Ev/Io.hpp"
#include"Jsmn/Object.hpp"
#include"Json/Out.hpp"
#include"S/Bus.hpp"
#include"Sqlite3.hpp"
#include"Stats/RunningMean.hpp"
#include"Util/make_unique.hpp"
#include<vector>

namespace {

/* Keep two weeks worth of data.  */
auto constexpr num_samples = std::size_t(2016);

auto const initialization = R"INIT(
DROP TABLE IF EXISTS "OnchainFeeMonitor_meanfee";
CREATE TABLE IF NOT EXISTS "OnchainFeeMonitor_samples"
	( id INTEGER PRIMARY KEY AUTOINCREMENT
	, data SAMPLE
	);
CREATE INDEX IF NOT EXISTS "OnchainFeeMonitor_samples_idx"
    ON "OnchainFeeMonitor_samples"(data);
)INIT";

/* Initial data we will load into OnchainFeeMonitor_samples,
 * so that new users have something reasonable to start out
 * with.  */
std::uint32_t const initial_sample_data[] =
{ 10429, 10430, 9967, 9968, 9968, 9969, 9969, 9969
, 9970, 9970, 9972, 9972, 9972, 9972, 9972, 9972
, 12541, 11444, 11444, 9973, 9974, 9178, 8155, 7475
, 7475, 7475, 7475, 12551, 11478, 11478, 11478, 8154
, 8154, 7441, 7444, 7444, 7444, 7444, 9996, 10397
, 11500, 11497, 11497, 11497, 11497, 14722, 14722, 14722
, 16278, 16278, 16278, 17007, 17000, 17000, 16295, 16295
, 16295, 16295, 16295, 16999, 16999, 16992, 16994, 16992
, 16997, 16997, 16997, 16997, 16316, 16315, 16316, 16315
, 16315, 16317, 16316, 16316, 16317, 16318, 16318, 16317
, 16317, 14685, 12584, 12578, 12578, 12578, 12578, 12578
, 12578, 16331, 16331, 12579, 12578, 12578, 12576, 12572
, 12567, 12567, 12564, 12562, 12562, 10976, 10976, 10976
, 10976, 10976, 10976, 10976, 10976, 10976, 10976, 10976
, 10976, 10976, 10976, 10976, 10976, 10975, 10976, 10965
, 10961, 10961, 10961, 10964, 10964, 10964, 9430, 9031
, 9031, 9031, 9031, 9031, 9031, 6761, 6765, 6765
, 6765, 6774, 6775, 6774, 6774, 6776, 6777, 6777
, 6777, 8181, 8181, 8181, 8181, 12613, 12611, 10385
, 9005, 6787, 6787, 5020, 5020, 5020, 5020, 5020
, 12588, 31960, 31950, 31940, 31937, 31956, 31956, 31981
, 30795, 30794, 30794, 30794, 30774, 30774, 30767, 30767
, 30767, 30767, 30779, 30687, 30687, 30687, 30687, 30687
, 30687, 33702, 33702, 70656, 70654, 70654, 70654, 70654
, 70654, 74397, 74397, 74397, 74394, 74394, 74391, 74391
, 74396, 74396, 74390, 74386, 74386, 70461, 70468, 70471
, 70479, 70479, 70487, 70500, 70502, 70502, 70531, 70531
, 70531, 70537, 70538, 70550, 70555, 70555, 70560, 70563
, 70563, 70563, 70563, 70563, 70563, 70563, 70564, 70564
, 70606, 70611, 66590, 66590, 70636, 66609, 66610, 66610
, 66610, 66625, 66625, 66622, 66624, 66624, 66596, 66586
, 66573, 66569, 66567, 66567, 66588, 66588, 66590, 66590
, 66637, 66675, 66672, 66668, 66668, 66668, 66753, 66753
, 60795, 60795, 60838, 57661, 49960, 47512, 47510, 47510
, 47497, 43170, 43170, 43170, 43170, 40964, 40964, 40967
, 40967, 40967, 40978, 35666, 35669, 33588, 33588, 33588
, 33608, 33608, 33620, 33613, 33609, 33609, 33609, 33609
, 33609, 33608, 35681, 35681, 35587, 35587, 35597, 35597
, 35597, 35597, 35605, 35608, 35608, 27891, 27891, 27891
, 27894, 27889, 27891, 27889, 27891, 27892, 27892, 27892
, 27892, 27892, 27892, 27892, 35719, 63272, 63272, 63323
, 63346, 63346, 63398, 63401, 63409, 63409, 63409, 63409
, 60937, 60937, 60937, 60938, 60938, 60939, 60939, 60941
, 57676, 57673, 57673, 57693, 57717, 57733, 57733, 57757
, 57771, 57771, 57771, 54847, 54857, 54868, 52156, 52156
, 52162, 52162, 52167, 52165, 52156, 52152, 52124, 52125
, 52125, 52109, 52106, 52106, 49946, 49946, 49968, 49966
, 49955, 49955, 49943, 49943, 37309, 33587, 32104, 32108
, 32107, 32109, 32108, 32110, 30538, 30541, 30541, 30527
, 30527, 30528, 30528, 30532, 30539, 30545, 30545, 30568
, 30592, 30592, 30592, 30592, 30592, 30592, 30592, 30637
, 30637, 30637, 30653, 30653, 30674, 30678, 30691, 30691
, 30691, 30691, 30691, 30691, 30691, 30691, 30691, 30693
, 27715, 27714, 27714, 26491, 26491, 12571, 4337, 4337
, 4337, 4334, 4334, 4330, 4330, 4330, 4330, 4330
, 4330, 4330, 18809, 17654, 12594, 12598, 12598, 12597
, 12596, 12596, 12596, 12596, 12101, 12101, 12099, 12099
, 12097, 12098, 12095, 12095, 12092, 12095, 7476, 7071
, 7071, 5539, 5266, 3982, 3982, 3982, 3982, 3764
, 2773, 2650, 2401, 2400, 2400, 2401, 2401, 2401
, 2401, 2402, 2402, 2402, 2402, 2012, 2012, 2012
, 1883, 1748, 1641, 1641, 1641, 1641, 1641, 1779
, 2010, 2010, 2010, 2232, 2008, 2008, 2008, 2008
, 2008, 2008, 2008, 2008, 2008, 2008, 2008, 2008
, 2008, 2008, 2008, 2008, 1883, 1883, 1884, 1884
, 1886, 1886, 1886, 1748, 1747, 1747, 1747, 1747
, 1747, 1747, 1747, 1885, 1747, 1747, 1746, 1746
, 1746, 1747, 1747, 1746, 1746, 1746, 1746, 1746
, 1746, 1746, 1746, 1746, 1746, 1746, 1747, 1747
, 1746, 1746, 1746, 1747, 1747, 1747, 1642, 1642
, 1642, 1642, 1547, 1547, 1547, 1547, 1503, 1503
, 1503, 1503, 1351, 1351, 1352, 1352, 887, 787
, 787, 787, 787, 754, 754, 754, 754, 754
, 754, 754, 754, 754, 754, 754, 754, 755
, 755, 754, 755, 755, 755, 755, 755, 755
, 755, 755, 755, 755, 755, 755, 755, 788
, 755, 755, 755, 755, 755, 755, 755, 755
, 755, 755, 755, 755, 788, 788, 788, 788
, 788, 788, 788, 5787, 5787, 8602, 8602, 10956
, 10956, 10943, 10419, 10422, 10427, 10427, 10429, 10425
, 10425, 10425, 10425, 10424, 10424, 10428, 10428, 10428
, 10431, 10431, 10432, 10008, 9011, 9011, 9015, 9017
, 9020, 9022, 9022, 9024, 8626, 8626, 8626, 6131
, 6134, 5794, 5794, 5794, 5794, 5794, 5794, 5794
, 5794, 5794, 5794, 5794, 5794, 5794, 5794, 5794
, 5794, 5794, 5794, 5797, 5814, 5273, 5277, 5277
, 5280, 4324, 4333, 4327, 4326, 4326, 4323, 4316
, 4316, 4316, 4316, 4315, 4314, 4314, 4314, 4141
, 4141, 4142, 4142, 4142, 3980, 3429, 3429, 3429
, 3430, 3430, 3430, 3432, 3433, 3433, 3433, 3435
, 2654, 2654, 2424, 2424, 2424, 1892, 1892, 1893
, 1545, 1545, 1545, 1750, 1750, 1750, 1750, 1750
, 1750, 1750, 1644, 1546, 1263, 1263, 1263, 1263
, 2950, 2949, 2651, 2770, 2770, 2770, 2953, 2953
, 2953, 19779, 18798, 18798, 18798, 18796, 18799, 18804
, 17065, 17067, 17072, 17072, 17080, 17083, 17083, 17080
, 3767, 3767, 3767, 4526, 4526, 4526, 4527, 4527
, 4527, 4527, 4527, 4770, 4770, 4767, 4767, 4767
, 4769, 4769, 4769, 4769, 4769, 4770, 4770, 4770
, 4770, 4771, 4771, 4772, 4772, 4522, 4522, 4522
, 4521, 4521, 4521, 4313, 4314, 4314, 4314, 4316
, 4316, 4316, 4316, 4097, 3985, 3985, 3985, 3986
, 3985, 3985, 3985, 3985, 3985, 3985, 3767, 3767
, 3767, 3767, 3767, 3767, 3767, 3767, 3767, 3767
, 3767, 3767, 3767, 3767, 3767, 3767, 3767, 3767
, 3767, 3767, 3767, 3767, 13342, 13342, 13342, 13355
, 12115, 10391, 10391, 10391, 10391, 10391, 10394, 10394
, 10402, 10402, 10402, 10406, 9434, 9434, 6768, 6768
, 6768, 6768, 6768, 6768, 5269, 5269, 5269, 5269
, 5269, 13968, 13952, 13952, 13953, 13951, 13950, 11513
, 11512, 11512, 11512, 11521, 10382, 10382, 10385, 11527
, 10389, 6772, 6772, 5271, 5271, 5271, 5271, 13361
, 13361, 23968, 23968, 27710, 27710, 27710, 27710, 27710
, 47493, 47479, 47479, 45063, 45062, 45062, 45108, 45112
, 45137, 45151, 45174, 45202, 45221, 45235, 45243, 45252
, 40992, 40992, 41008, 35350, 35350, 35351, 35351, 35363
, 35363, 35391, 35391, 35430, 35439, 33767, 33767, 30542
, 30550, 30550, 30579, 30580, 30583, 30583, 30583, 29175
, 29162, 29162, 29154, 29154, 29154, 29166, 29166, 29200
, 27821, 27827, 27829, 27836, 27842, 27842, 27843, 27849
, 27849, 27849, 27880, 27880, 24038, 24042, 24042, 24049
, 24049, 24049, 24049, 24077, 24087, 24098, 24098, 24122
, 24122, 21733, 21739, 21752, 21752, 21765, 21780, 21796
, 21799, 21799, 21814, 21814, 18795, 18796, 18796, 18809
, 18810, 18812, 18816, 18816, 18816, 18816, 18816, 18818
, 18818, 18818, 18818, 20671, 19735, 19737, 19737, 19737
, 19750, 19757, 19757, 19757, 19757, 19763, 19763, 17723
, 17723, 17723, 16096, 16098, 16099, 13353, 13353, 13353
, 13353, 9901, 9901, 9901, 9902, 14836, 17119, 17125
, 17125, 17125, 17125, 17734, 19791, 17728, 17728, 17728
, 10429, 9907, 9907, 9907, 9895, 9893, 9892, 9890
, 9890, 9890, 9890, 9890, 9890, 9890, 9890, 26446
, 26447, 26446, 22797, 21775, 21779, 21779, 21786, 21786
, 21794, 21795, 21798, 21800, 21802, 21806, 21806, 21809
, 21809, 21809, 10405, 10407, 10408, 9430, 8657, 8656
, 8656, 8656, 8657, 8658, 8658, 8657, 8656, 8657
, 8659, 8659, 8658, 8658, 8658, 8658, 8659, 8659
, 8665, 8656, 8657, 8657, 8657, 8661, 8661, 8661
, 8661, 8663, 8663, 8666, 8669, 8669, 8670, 7064
, 7065, 7063, 7063, 7063, 7062, 7062, 7063, 7063
, 7065, 1903, 754, 754, 754, 754, 754, 902
, 1414, 1501, 1501, 1781, 1889, 1889, 1889, 1889
, 4107, 4107, 3969, 3969, 3968, 3968, 3968, 3968
, 4778, 4778, 4539, 4539, 4539, 4539, 4539, 4539
, 5271, 6409, 6409, 6409, 6409, 6409, 8653, 9962
, 13999, 14002, 14002, 14008, 14008, 14015, 14018, 14023
, 14026, 14031, 14031, 14051, 13287, 13294, 13295, 13295
, 13295, 13323, 13324, 13327, 13327, 13334, 13334, 13334
, 13334, 13334, 13334, 14063, 14061, 14061, 14062, 14061
, 14061, 13364, 13365, 13365, 13366, 13366, 13366, 13366
, 13366, 13366, 14056, 14057, 14057, 14058, 14058, 14058
, 14058, 14063, 14063, 14063, 14066, 12097, 12092, 12092
, 12089, 12089, 10994, 10994, 10994, 10994, 10990, 10990
, 10983, 10985, 10985, 9949, 9948, 9948, 9948, 9948
, 9948, 10989, 10994, 10995, 10993, 8179, 8179, 8169
, 8169, 8167, 8167, 8167, 8167, 8167, 8163, 8163
, 8161, 8162, 8163, 8165, 8164, 8164, 8164, 8169
, 5009, 2655, 2655, 2655, 1052, 1052, 1053, 1053
, 1054, 1054, 1054, 1109, 1109, 1263, 1264, 1264
, 1264, 1264, 1264, 1108, 1108, 1108, 1108, 1108
, 1264, 1264, 1264, 1264, 1264, 1264, 1264, 1264
, 2673, 3763, 3763, 3763, 3764, 3765, 3765, 3765
, 3765, 1746, 1746, 1746, 1747, 1747, 1747, 2214
, 3538, 3538, 5009, 5009, 5009, 5012, 5012, 5012
, 4780, 4780, 4781, 4782, 4782, 4782, 4782, 4782
, 4782, 4782, 5269, 9952, 9952, 11015, 10407, 10407
, 10406, 10404, 9925, 9925, 9925, 9925, 9912, 9912
, 9912, 9912, 9912, 9912, 11008, 13338, 13338, 26626
, 26626, 26627, 26628, 26628, 26630, 26630, 26633, 26635
, 26636, 26636, 26640, 26640, 26640, 26641, 26642, 26642
, 26648, 26648, 14802, 14802, 14796, 14788, 14021, 13353
, 13353, 13357, 13356, 13356, 13356, 13356, 13358, 13358
, 13358, 13359, 13359, 13359, 13361, 13359, 13358, 13358
, 12614, 12614, 12616, 11472, 11472, 11477, 11479, 11479
, 11483, 11483, 11010, 11010, 11010, 11011, 11009, 11009
, 11011, 11015, 11015, 10423, 10424, 10420, 10417, 9955
, 9955, 9026, 9026, 9026, 9026, 9026, 9038, 9039
, 9036, 9040, 9040, 9040, 9040, 9040, 8616, 8616
, 8614, 8138, 8137, 8137, 8137, 7818, 7818, 7818
, 7818, 7821, 7822, 7822, 7822, 7822, 7823, 7823
, 7823, 7830, 7832, 7832, 7832, 7833, 5271, 1188
, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056
, 2078, 7494, 7494, 7493, 7493, 7494, 7494, 7494
, 7494, 7492, 7492, 7492, 7493, 7493, 5016, 5016
, 5016, 5016, 5016, 3265, 2072, 2074, 2007, 2007
, 2007, 2007, 1880, 1879, 1879, 1879, 1879, 1879
, 1879, 2007, 1879, 1878, 1878, 1878, 1878, 1878
, 1878, 1878, 2006, 2006, 2005, 2005, 2005, 1878
, 1746, 1746, 1746, 1746, 1746, 1746, 1746, 1746
, 1746, 1746, 2008, 1745, 1745, 1745, 1744, 1744
, 1745, 1745, 1745, 1643, 1554, 1554, 1553, 1553
, 1553, 1553, 1553, 1553, 1553, 1553, 1553, 1554
, 1346, 1346, 1345, 1345, 1345, 1346, 1054, 1054
, 1054, 1054, 963, 789, 789, 789, 789, 789
, 789, 789, 789, 789, 789, 789, 789, 789
, 789, 789, 789, 789, 789, 789, 789, 789
, 789, 789, 789, 789, 789, 789, 789, 789
, 789, 905, 1055, 1055, 1055, 1055, 1055, 1055
, 1055, 1055, 1055, 1055, 1055, 1557, 1242, 1174
, 1174, 1174, 1174, 1174, 1174, 1501, 1556, 1777
, 1777, 1891, 1891, 2008, 2008, 2008, 2008, 2008
, 2008, 2008, 2091, 2402, 2400, 2400, 2400, 2400
, 2400, 2401, 2401, 2401, 2402, 2402, 2403, 2403
, 2403, 2404, 2404, 2406, 2406, 2407, 2408, 2408
, 2409, 2409, 2410, 2411, 2412, 2412, 2412, 2412
, 2415, 2415, 2417, 2418, 2418, 2418, 2418, 2421
, 2421, 2421, 2422, 2424, 2427, 2277, 2009, 2009
, 2009, 2009, 2009, 2009, 2009, 2009, 2009, 2009
, 2009, 2009, 2009, 2008, 2007, 2007, 2007, 2007
, 2007, 2007, 1894, 1894, 1894, 1894, 1894, 279
, 279, 279, 279, 269, 269, 269, 269, 755
, 755, 755, 2097, 2097, 2097, 1006, 788, 755
, 755, 755, 755, 755, 755, 788, 1054, 1055
, 1055, 1055, 1234, 1234, 1263, 1263, 1241, 1241
, 1241, 1240, 1240, 1240, 1348, 1348, 1348, 1346
, 1346, 1346, 1346, 1346, 1346, 1346, 10380, 10380
, 10392, 10392, 10392, 10393, 10392, 10395, 10395, 10396
, 10396, 10977, 10971, 10971, 10972, 13987, 13987, 13987
, 13987, 16974, 16967, 16972, 16978, 16986, 16992, 16996
, 16996, 17006, 17012, 17013, 17021, 17026, 17032, 17032
, 17046, 17050, 17057, 17064, 17064, 17064, 17065, 17065
, 17065, 17751, 17751, 17751, 18826, 17704, 17697, 17696
, 17696, 17698, 17698, 17698, 17701, 17704, 17704, 17704
, 17712, 17714, 17714, 17714, 17726, 17720, 17723, 17727
, 17731, 17740, 17740, 17740, 17744, 17744, 17744, 17758
, 17758, 17758, 17760, 17761, 17761, 17761, 17788, 17792
, 17786, 17787, 17787, 17797, 17802, 17807, 17808, 17809
, 17809, 17809, 17808, 17808, 17814, 17816, 17816, 17816
, 17830, 17830, 17830, 17844, 17844, 17857, 17861, 17863
, 17863, 17867, 17873, 17875, 17875, 17884, 17886, 17891
, 17896, 17898, 17900, 17900, 11843, 9039, 8187, 8187
, 8187, 8187, 8187, 8187, 8187, 8187, 8187, 10460
, 11523, 10448, 10452, 9952, 9952, 9952, 7479, 7478
, 7479, 7479, 7479, 7479, 12602, 13318, 11972, 11972
, 11972, 11972, 15555, 15555, 15555, 15555, 16177, 17957
, 17957, 17960, 17961, 17962, 17963, 17963, 17963, 17963
, 17977, 17977, 17984, 17984, 17984, 17984, 17984, 17984
, 17984, 18701, 20633, 20632, 20642, 20642, 20652, 20657
, 20660, 20662, 20662, 20667, 20663, 20671, 20671, 20671
, 20690, 20690, 20690, 20690, 20690, 20714, 20694, 20694
, 20700, 20702, 20702, 20702, 20710, 20712, 20713, 20713
, 20716, 20716, 20721, 20722, 20723, 20724, 20724, 20722
, 20723, 20725, 20728, 20728, 20726, 20726, 20726, 20726
, 20726, 20730, 20729, 20730, 20730, 20730, 20734, 20734
, 20735, 20734, 19833, 19837, 19843, 19844, 19848, 19848
, 19848, 19848, 19863, 19867, 19867, 19874, 19874, 19886
, 19892, 19886, 19884, 19884, 19884, 19884, 19885, 19886
, 19886, 19887, 18750, 18751, 18761, 18768, 18779, 18786
, 18786, 18801, 18801, 18812, 18818, 18826, 18832, 18833
, 18835, 18839, 18843, 18843, 18843, 11030, 11030, 11031
, 11031, 11030, 5272, 4781, 3963, 3764, 3550, 3550
, 3764, 8661, 8661, 12633, 12640, 12659, 12660, 12659
};

static_assert( num_samples == ( sizeof(initial_sample_data)
			      / sizeof(initial_sample_data[0])
			      )
	     , "initial_sample_data number of entries must equal num_samples"
	     );

/* What percentile to use to judge low and high.
 * `mid_percentile` is used in the initial starting check.
 * `hi_to_lo_percentile` is used when we are currently in
 * "high fees" mode, and if the onchain fees get less than
 * or equal that rate, we move to "low fees" mode.
 * `lo_to_hi_percentile` is used when we are currently in
 * "low fees" mode.
 */
auto constexpr hi_to_lo_percentile = double(20);
auto constexpr mid_percentile = double(25);
auto constexpr lo_to_hi_percentile = double(30);

}

namespace Boss { namespace Mod {

class OnchainFeeMonitor::Impl {
private:
	S::Bus& bus;
	Boss::Mod::Waiter& waiter;
	Boss::Mod::Rpc *rpc;
	Sqlite3::Db db;

	bool is_low_fee_flag;
	std::unique_ptr<double> last_feerate;

	void start() {
		bus.subscribe<Msg::DbResource
			     >([this](Msg::DbResource const& m) {
			db = m.db;
			/* Set up table if not yet present.  */
			return db.transact().then([this](Sqlite3::Tx tx) {
				tx.query_execute(initialization);
				initialize_sample_data(tx);
				tx.commit();
				return Ev::lift();
			});
		});
		bus.subscribe<Msg::Init>([this](Msg::Init const& ini) {
			rpc = &ini.rpc;
			return Boss::concurrent(on_init());
		});
		bus.subscribe<Msg::Timer10Minutes>([this](Msg::Timer10Minutes const&) {
			return on_timer();
		});
		/* Report status.  */
		bus.subscribe<Msg::SolicitStatus
			     >([this](Msg::SolicitStatus const& _) {
			if (!db)
				return Ev::lift();
			return db.transact().then([this](Sqlite3::Tx tx) {
				auto h2l = get_feerate_at_percentile(
					tx, hi_to_lo_percentile
				);
				auto mid = get_feerate_at_percentile(
					tx, mid_percentile
				);
				auto l2h = get_feerate_at_percentile(
					tx, lo_to_hi_percentile
				);
				tx.commit();

				auto status = Json::Out()
					.start_object()
						.field("hi_to_lo", h2l)
						.field("init_mid", mid)
						.field("lo_to_hi", l2h)
						.field( "last_feerate_perkw"
						      , last_feerate ? *last_feerate : -1.0
						      )
						.field( "judgment"
						      , std::string(is_low_fee_flag ? "low fees" : "high fees")
						      )
					.end_object()
					;
				return bus.raise(Msg::ProvideStatus{
					"onchain_feerate",
					std::move(status)
				});
			});
		});
	}
	void initialize_sample_data(Sqlite3::Tx& tx) {
		auto cnt = tx.query(R"QRY(
		SELECT COUNT(*) FROM "OnchainFeeMonitor_samples";
		)QRY").execute();
		auto count = std::size_t();
		for (auto& r : cnt)
			count = r.get<std::size_t>(0);
		if (count >= num_samples)
			return;
		for (auto i = count; i < num_samples; ++i) {
			tx.query(R"QRY(
			INSERT INTO "OnchainFeeMonitor_samples"(data)
			VALUES(:data);
			)QRY")
				.bind(":data", initial_sample_data[i])
				.execute()
				;
		}
	}

	Ev::Io<void> on_init() {
		auto saved_feerate = std::make_shared<double>();

		return Ev::lift().then([this]() {
			/* Now query the feerate.  */
			return get_feerate();
		}).then([ this
			, saved_feerate
			](std::unique_ptr<double> feerate) {
			/* If feerate is unknown, just assume we are at
			 * a high-feerate time.
			 */
			if (!feerate)
				return Boss::log( bus, Debug
						, "OnchainFeeMonitor: Init: "
						  "Fee unknown."
						).then([this]() {
					return Boss::concurrent(retry());
				});

			/* Feerate is known, save it in the shared variable.  */
			*saved_feerate = *feerate;
			last_feerate = std::move(feerate);

			/* Now access the db and check if it is lower or
			 * higher than the mid percentile, to know our current
			 * low/high flag.  */
			return db.transact().then([ this
						  , saved_feerate
						  ](Sqlite3::Tx tx) {
				auto mid = get_feerate_at_percentile(
					tx, mid_percentile
				);
				tx.commit();
				is_low_fee_flag = *saved_feerate < mid;
				return report_fee("Init");
			});
		});
	}

	/* Use std::unique_ptr<double> as a sort of Optional Real.  */
	Ev::Io<std::unique_ptr<double>> get_feerate() {
		return rpc->command("feerates"
				   , Json::Out()
					.start_object()
						.field("style", std::string("perkw"))
					.end_object()
				   ).then([](Jsmn::Object res) {
			auto failed = []() {
				return Ev::lift<std::unique_ptr<double>>(
					nullptr
				);
			};
			if (!res.is_object() || !res.has("perkw"))
				return failed();
			auto perkw = res["perkw"];
			if (!perkw.is_object() || !perkw.has("opening"))
				return failed();
			auto opening = perkw["opening"];
			if (!opening.is_number())
				return failed();
			auto value = (double) opening;
			return Ev::lift(
				Util::make_unique<double>(value)
			);
		});
	}

	void add_sample(Sqlite3::Tx& tx, double feerate_sample) {
		tx.query(R"QRY(
		INSERT INTO "OnchainFeeMonitor_samples"
		      ( data)
		VALUES(:data);
		)QRY")
			.bind(":data", feerate_sample)
			.execute()
			;

		check_num_samples(tx);
	}
	void check_num_samples(Sqlite3::Tx& tx) {
		/* Did we get above `num_samples`?  */
		auto fetch = tx.query(R"QRY(
		SELECT COUNT(*)
		  FROM "OnchainFeeMonitor_samples"
		     ;
		)QRY")
			.execute()
			;
		auto num = std::size_t();
		for (auto& r : fetch)
			num = r.get<std::size_t>(0);

		if (num > num_samples) {
			/* We did!  Delete old ones.  */

			/** FIXME: This could have been done with
			 * `DELETE ... ORDER BY id LIMIT :limit`,
			 * but not all OSs (presumably FreeBSD or
			 * MacOS) have a default SQLITE3 with
			 * `SQLITE_ENABLE_UPDATE_DELETE_LIMIT`
			 * enabled.
			 * The real fix is to put SQLITE3 into our
			 * `external/` dir where we can strictly
			 * control the flags it gets compiled
			 * with.
			 */
			auto ids = std::vector<std::uint64_t>();
			auto fetch = tx.query(R"QRY(
			SELECT id FROM "OnchainFeeMonitor_samples"
			 ORDER BY id
			 LIMIT :limit
			     ;
			)QRY")
				.bind(":limit", num - num_samples)
				.execute()
				;
			for (auto& r : fetch)
				ids.push_back(r.get<std::uint64_t>(0));
			for (auto id : ids) {
				tx.query(R"QRY(
				DELETE FROM "OnchainFeeMonitor_samples"
				 WHERE id = :id
				     ;
				)QRY")
					.bind(":id", id)
					.execute()
					;
			}
		}
	}

	double
	get_feerate_at_percentile(Sqlite3::Tx& tx, double percentile) {
		auto index = std::size_t(
			double(num_samples) * percentile / 100.0
		);
		if (index < 0)
			index = std::size_t(0);
		else if (index >= num_samples)
			index = num_samples - 1;

		auto fetch = tx.query(R"QRY(
		SELECT data FROM "OnchainFeeMonitor_samples"
		 ORDER BY data
		 LIMIT 1 OFFSET :index
		)QRY")
			.bind(":index", index)
			.execute();
		auto rv = double();
		for (auto& r : fetch)
			rv = r.get<double>(0);

		return rv;
	}

	Ev::Io<void> report_fee(char const* msg) {
		return Boss::log( bus, Debug
				, "OnchainFeeMonitor: %s: %s fees."
				, msg
				, is_low_fee_flag ? "low" : "high"
				).then([this]() {
			return bus.raise(Msg::OnchainFee{is_low_fee_flag});
		});
	}

	Ev::Io<void> on_timer() {
		auto saved_feerate = std::make_shared<double>();

		return get_feerate().then([ this
					  , saved_feerate
					  ](std::unique_ptr<double> feerate) {
			if (!feerate)
				return report_fee("Periodic: fee unknown, retaining");

			*saved_feerate = *feerate;
			last_feerate = std::move(feerate);

			return db.transact().then([ this
						  , saved_feerate
						  ](Sqlite3::Tx tx) {
				add_sample(tx, *saved_feerate);

				/* Apply hysteresis.  */
				if (is_low_fee_flag) {
					auto ref = get_feerate_at_percentile(
						tx, lo_to_hi_percentile
					);
					if (*saved_feerate >= ref)
						is_low_fee_flag = false;
				} else {
					auto ref = get_feerate_at_percentile(
						tx, hi_to_lo_percentile
					);
					if (*saved_feerate <= ref)
						is_low_fee_flag = true;
				}
				tx.commit();
				return report_fee("Periodic");
			});
		});
	}

	/* Loop until we get fees.  */
	Ev::Io<void> retry() {
		auto saved_feerate = std::make_shared<double>();
		return waiter.wait(30).then([this]() {
			return get_feerate();
		}).then([this, saved_feerate
			](std::unique_ptr<double> feerate) {
			if (!feerate)
				return Boss::log( bus, Debug
						, "OnchainFeeMonitor: "
						  "Init retried: "
						  "Fee still unknown."
						).then([this]() {
					return retry();
				});

			return db.transact().then([ this
						  , saved_feerate
						  ](Sqlite3::Tx tx) {
				auto mid = get_feerate_at_percentile(
					tx, mid_percentile
				);
				tx.commit();
				is_low_fee_flag = *saved_feerate < mid;
				return report_fee("Init");
			});
		});
	}

public:
	explicit
	Impl( S::Bus& bus_
	    , Boss::Mod::Waiter& waiter_
	    ) : bus(bus_), waiter(waiter_)
	      , rpc(nullptr), is_low_fee_flag(false)
	      , last_feerate(nullptr)
	      {
		start();
	}

	bool is_low_fee() const {
		return is_low_fee_flag;
	}
};

OnchainFeeMonitor::OnchainFeeMonitor( S::Bus& bus
				    , Boss::Mod::Waiter& waiter
				    )
	: pimpl(Util::make_unique<Impl>(bus, waiter)) { }
OnchainFeeMonitor::OnchainFeeMonitor(OnchainFeeMonitor&& o)
	: pimpl(std::move(o.pimpl)) { }
OnchainFeeMonitor::~OnchainFeeMonitor() { }

bool OnchainFeeMonitor::is_low_fee() const {
	return pimpl->is_low_fee();
}

}}
