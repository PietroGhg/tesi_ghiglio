# Reference:
https://github.com/MaybeShewill-CV/lanenet-lane-detection
# Train:
python tools/train_lanenet.py --dataset_dir ./data/training --net_flag vgg -m 0
# Inference:
python tools/evaluate_lanenet_on_tusimple.py --weights_path ./model/tusimple_lanenet_vgg/tusimple_lanenet_vgg_2019-08-20-20-12-46.ckpt-10001 --image_dir ./data/test_data/clips --save_dir test_output
# Send model folder from cloud to edge
