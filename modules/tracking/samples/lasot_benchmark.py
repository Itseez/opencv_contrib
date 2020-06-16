import numpy as np
import cv2 as cv
import argparse
import os


def get_iou(new, gt):
    '''
    During the calculation of intersection over union, we are checking
    numerical value of area_of_overlap, because if it is equal to 0,
    we have no intersection.
    '''
    new_xmin = new[0]
    new_xmax = new[0] + new[2] - 1.0
    new_ymin = new[1]
    new_ymax = new[1] + new[3] - 1.0
    gt_xmin = gt[0]
    gt_xmax = gt[0] + gt[2] - 1.0
    gt_ymin = gt[1]
    gt_ymax = gt[1] + gt[3] - 1.0
    dx = max(0, min(new_xmax, gt_xmax) - max(new_xmin, gt_xmin))
    dy = max(0, min(new_ymax, gt_ymax) - max(new_ymin, gt_ymin))
    area_of_overlap = dx * dy
    area_of_union = (new_xmax - new_xmin) * (new_ymax - new_ymin) + (
        gt_xmax - gt_xmin) * (gt_ymax - gt_ymin) - area_of_overlap
    if area_of_overlap != 0 and area_of_union != 0:
        iou = area_of_overlap / area_of_union
    else:
        iou = 0
    return iou


def get_pr(new, gt, is_norm):
    '''
    In calculations of precision and normalized precision are used thresholds
    from original TrackingNet paper.
    '''
    new_cx = new[0] + (new[2] + 1.0) / 2
    new_cy = new[1] + (new[3] + 1.0) / 2
    gt_cx = gt[0] + (gt[2] + 1.0) / 2
    gt_cy = gt[1] + (gt[3] + 1.0) / 2
    gt_bb_w = gt[2]
    gt_bb_h = gt[3]
    dx = new_cx - gt_cx
    dy = new_cy - gt_cy
    if is_norm:
        dx /= gt_bb_w
        dy /= gt_bb_h
    return np.sqrt(dx ** 2 + dy ** 2)


def init_tracker(tracker_name):
    '''
    Method used for initializing of trackers by creating it
    via cv.TrackerX_create().
    Input: string with tracker name.
    Output: dictionary 'config'
    Dictionary 'config' contains trackers names
    as keys and tuple with call method and number of frames for
    reinitialization as values
    '''
    config = {"Boosting": (cv.TrackerBoosting_create(), 500),
              "MIL": (cv.TrackerMIL_create(), 1000),
              "KCF": (cv.TrackerKCF_create(), 1000),
              "MedianFlow": (cv.TrackerMedianFlow_create(), 1000),
              "GOTURN": (cv.TrackerGOTURN_create(), 250),
              "MOSSE": (cv.TrackerMOSSE_create(), 1000),
              "CSRT": (cv.TrackerCSRT_create(), 1000)}
    return config[tracker_name]


def main():
    parser = argparse.ArgumentParser(
        description="Run LaSOT-based benchmark for visual object trackers")
    # As a default argument used name of
    # original dataset folder
    parser.add_argument("--dataset", type=str,
                        default="LaSOTTesting", help="Full path to LaSOT")
    parser.add_argument("--v", dest="visualization", action='store_true',
                        help="Showing process of tracking")
    args = parser.parse_args()

    # Creating list with names of videos via reading names from txt file
    video_names = os.path.join(args.dataset, "testing_set.txt")
    with open(video_names, 'rt') as f:
        list_of_videos = f.read().rstrip('\n').split('\n')
    trackers = [
        'Boosting', 'MIL', 'KCF', 'MedianFlow', 'GOTURN', 'MOSSE', 'CSRT']

    iou_avg = []
    pr_avg = []
    n_pr_avg = []

    # Loop for every tracker
    for tracker_name in trackers:

        print("Tracker name: ", tracker_name)

        number_of_thresholds = 21
        iou_video = np.zeros(number_of_thresholds)
        pr_video = np.zeros(number_of_thresholds)
        n_pr_video = np.zeros(number_of_thresholds)
        iou_thr = np.linspace(0, 1, number_of_thresholds)
        pr_thr = np.linspace(0, 50, number_of_thresholds)
        n_pr_thr = np.linspace(0, 0.5, number_of_thresholds)

        # Loop for every video
        for video_name in list_of_videos:

            tracker, frames_for_reinit = init_tracker(tracker_name)
            init_once = False

            print("\tVideo name: " + str(video_name))

            # Open specific video and read ground truth for it
            gt_file = open(os.path.join(args.dataset, video_name,
                                        "groundtruth.txt"), "r")
            gt_bb = gt_file.readline().rstrip("\n").split(",")
            init_bb = tuple([float(b) for b in gt_bb])

            # Creating blob from image sequence
            video_sequence = sorted(os.listdir(os.path.join(
                args.dataset, video_name, "img")))

            # Variables for saving sum of every metric for every frame and
            # every video respectively
            iou_values = []
            pr_values = []
            n_pr_values = []
            frame_counter = len(video_sequence)

            # For every frame in video
            for number_of_the_frame, image in enumerate(video_sequence):
                frame = cv.imread(os.path.join(
                    args.dataset, video_name, "img", image))
                gt_bb = tuple([float(x) for x in gt_bb])

                # Check for presence of object on the frame
                # If no object on frame, we ignoring that frame
                if gt_bb[2] == 0 or gt_bb[3] == 0:
                    gt_bb = gt_file.readline().rstrip("\n").split(",")
                    continue

                # Condition of tracker`s re-initialization
                if ((number_of_the_frame + 1) % frames_for_reinit == 0):

                    tracker, frames_for_reinit = init_tracker(tracker_name)
                    init_once = False
                    init_bb = gt_bb

                if not init_once:
                    init_state = tracker.init(frame, init_bb)
                    init_once = True
                init_state, new_bb = tracker.update(frame)

                if args.visualization:
                    cv.rectangle(frame, (int(new_bb[0]), int(new_bb[1])), (int(
                        new_bb[0] + new_bb[2]), int(new_bb[1] + new_bb[3])), (
                            200, 0, 0))
                    cv.imshow("Tracking", frame)
                    cv.waitKey(1)

                iou_values.append(get_iou(new_bb, gt_bb))
                pr_values.append(get_pr(new_bb, gt_bb, False))
                n_pr_values.append(get_pr(new_bb, gt_bb, True))

                # Setting as ground truth bounding box from next frame
                gt_bb = gt_file.readline().rstrip("\n").split(",")

            # Calculating mean arithmetic value for specific video
            iou_video += (np.fromiter([sum(i >= thr for i in iou_values).astype(
                float) / frame_counter for thr in iou_thr], dtype=float))
            pr_video += (np.fromiter([sum(i <= thr for i in pr_values).astype(
                float) / frame_counter for thr in pr_thr], dtype=float))
            n_pr_video += (np.fromiter([sum(i <= thr for i in n_pr_values).astype(
                float) / frame_counter for thr in n_pr_thr], dtype=float))

        iou_mean_avg = np.array(iou_video) / len(list_of_videos)
        pr_mean_avg = np.array(pr_video) / len(list_of_videos)
        n_pr_mean_avg = np.array(n_pr_video) / len(list_of_videos)

        # We find the area under the curve according to the trapezoid rule 
        # and normalize by the maximum threshold value
        iou = np.trapz(iou_mean_avg, x=iou_thr) / iou_thr[-1]
        pr = np.trapz(pr_mean_avg, x=pr_thr) / pr_thr[-1]
        n_pr = np.trapz(n_pr_mean_avg, x=n_pr_thr) / n_pr_thr[-1]

        iou_avg.append('%.4f' % iou)
        pr_avg.append('%.4f' % pr)
        n_pr_avg.append('%.4f' % n_pr)

    titles = ["Names:", "IoU:", "Precision:", "N.Precision:"]
    data = [titles] + list(zip(trackers, iou_avg, pr_avg, n_pr_avg))
    for number, for_tracker in enumerate(data):
        line = '|'.join(str(x).ljust(20) for x in for_tracker)
        print(line)
        if number == 0:
            print('-' * len(line))


if __name__ == '__main__':
    main()
