import cv2
import numpy as np
import logging
import datetime
import configparser
import os
#from model import ComboBOX

def configuration():
    config_file_path = "C:/Users\hyewo\Desktop\C++\machinevision\config_model.ini"
    config = configparser.ConfigParser()
    config.read(config_file_path)

    global tae
    tae = 0

    #target_model = ComboBOX.showText(str)
    target_model = "GALAXY S21"

    coordinates = []

    if config.has_section(target_model):

        for key in ['camera', 'corner1', 'corner2', 'corner3', 'corner4']:
            x_key = f'{key}_x'
            y_key = f'{key}_y'

            if config.has_option(target_model, x_key) and config.has_option(target_model, y_key):
                x = int(config.get(target_model, x_key))
                y = int(config.get(target_model, y_key))
                coordinates.append((x, y))
            else:
                print(f"{target_model}에 대한 {key} 좌표가 설정 파일에 없습니다.")
                tae = 10
    else:
        print(f"{target_model}에 대한 정보가 설정 파일에 없습니다.")
        tae = 11

    return coordinates, tae, target_model

def threshold(img):
    center = img[int(img.shape[0] * 0.45):int(img.shape[0] * 0.55), int(img.shape[1] * 0.45):int(img.shape[1] * 0.55)]
    corner1 = img[0:int(img.shape[0] * 0.025), 0:int(img.shape[1] * 0.025)]
    corner2 = img[0:int(img.shape[0] * 0.025), int(img.shape[1] * 0.975):img.shape[1]]
    corner3 = img[int(img.shape[0] * 0.975):img.shape[0], 0:int(img.shape[1] * 0.025)]
    corner4 = img[int(img.shape[0] * 0.975):img.shape[0], int(img.shape[1] * 0.975):img.shape[1]]
    threshold_value = int((int(np.mean(center)) + int(np.mean(corner1)) + int(np.mean(corner2)) + int(
        np.mean(corner3)) + int(np.mean(corner4))) / 5)
    threshold_value *= 3
    _, img_t = cv2.threshold(img, threshold_value, 255, cv2.THRESH_BINARY)
    return img_t


def get_crosspt(x11, y11, x12, y12, x21, y21, x22, y22):  # 두 직선의 교점을 구함
    if x12 == x11 or x22 == x21:
        if x12 == x11:
            cx = x12
            m2 = (y22 - y21) / (x22 - x21)
            cy = m2 * (cx - x21) + y21
            return (int)(cx), (int)(cy)
        if x22 == x21:
            cx = x22
            m1 = (y12 - y11) / (x12 - x11)
            cy = m1 * (cx - x11) + y11
            return (int)(cx), (int)(cy)

    m1 = (y12 - y11) / (x12 - x11)
    m2 = (y22 - y21) / (x22 - x21)
    if m1 == m2:
        return None
    cx = (x11 * m1 - y11 - x21 * m2 + y21) / (m1 - m2)
    cy = m1 * (cx - x11) + y11

    return (int)(cx), (int)(cy)


def deskew(img, img_color):  # 디스플레이 영역 검출

    width_img = img.shape[1]  # 이미지의 너비
    height_img = img.shape[0]  # 이미지의 높이

    canny = cv2.Canny(img, 5000, 1500, apertureSize=5, L2gradient=True)  # 에지 영상 획득
    lines = cv2.HoughLinesP(canny, 0.8, np.pi / 180, 50, minLineLength=height_img * 0.01,
                            maxLineGap=height_img * 0.2)  # 에지 영상에서 선 검출
    left_lines, right_lines, top_lines, bottom_lines = [], [], [], []
    if lines is None:
        return 0
    for i in lines:
        ini = [i[0][0], i[0][1]]  # 선분의 시작점
        fini = [i[0][2], i[0][3]]  # 선분의 끝점
        x = fini[0] - ini[0]  # x 증가량
        y = fini[1] - ini[1]  # y 증가량

        if x < 0:
            x = -x
        if y < 0:
            y = -y

        # 선분의 중심 위치와 기울기에 따라 상, 하, 좌, 우 구분
        if (((ini[0] + fini[0]) / 2) < width_img * 0.5) and y > x:
            left_lines.append(ini)
            left_lines.append(fini)
        elif (((ini[0] + fini[0]) / 2) > width_img * 0.5) and y > x:
            right_lines.append(ini)
            right_lines.append(fini)
        elif (((ini[1] + fini[1]) / 2) < height_img * 0.5) and x > y:
            top_lines.append(ini)
            top_lines.append(fini)
        elif (((ini[1] + fini[1]) / 2) > height_img * 0.5) and x > y:
            bottom_lines.append(ini)
            bottom_lines.append(fini)

    index = []  # 선분 리스트에서 제거할 index
    count = 0
    for i in range(0, len(left_lines), 2):
        x = (int)((left_lines[i][0] + left_lines[i + 1][0]) / 2)  # 선분 중심점 x좌표
        y = (int)((left_lines[i][1] + left_lines[i + 1][1]) / 2)  # 선분 중심점 y좌표
        for j in range(3, (int)(width_img * 0.1)):  # 디스플레이 안쪽의 선분인지 판별
            if (x - j) >= 0:
                if (img[y, x - j]) == 255:
                    count = count + 1
        if (count >= 3):  # 흰 픽셀이 3개 이상이면 안쪽으로 간주
            count = 0
            index.append(i)

    for i in reversed(index):  # 디스플레이 안쪽에 해당하는 선분
        del left_lines[i]  # 리스트에서 제거
        del left_lines[i]

    index = []
    for i in range(0, len(right_lines), 2):
        x = (int)((right_lines[i][0] + right_lines[i + 1][0]) / 2)  # 선분 중심점 x좌표
        y = (int)((right_lines[i][1] + right_lines[i + 1][1]) / 2)  # 선분 중심점 y좌표
        for j in range(3, (int)(width_img * 0.1)):  # 디스플레이 안쪽의 선분인지 판별
            if (x + j) < width_img:
                if (img[y, x + j]) == 255:
                    count = count + 1
        if (count >= 3):  # 흰 픽셀이 3개 이상이면 안쪽으로 간주
            count = 0
            index.append(i)
    for i in reversed(index):  # 디스플레이 안쪽에 해당하는 선분
        del right_lines[i]  # 리스트에서 제거
        del right_lines[i]

    index = []
    for i in range(0, len(top_lines), 2):
        x = (int)((top_lines[i][0] + top_lines[i + 1][0]) / 2)  # 선분 중심점 x좌표
        y = (int)((top_lines[i][1] + top_lines[i + 1][1]) / 2)  # 선분 중심점 y좌표
        for j in range(3, (int)(height_img * 0.1)):  # 디스플레이 안쪽의 선분인지 판별
            if (y - j) >= 0:
                if (img[y - j, x]) == 255:
                    count = count + 1
        if (count >= 3):  # 흰 픽셀이 3개 이상이면 안쪽으로 간주
            count = 0
            index.append(i)
    for i in reversed(index):  # 디스플레이 안쪽에 해당하는 선분
        del top_lines[i]  # 리스트에서 제거
        del top_lines[i]

    index = []
    for i in range(0, len(bottom_lines), 2):
        x = (int)((bottom_lines[i][0] + bottom_lines[i + 1][0]) / 2)  # 선분 중심점 x좌표
        y = (int)((bottom_lines[i][1] + bottom_lines[i + 1][1]) / 2)  # 선분 중심점 y좌표
        for j in range(3, (int)(height_img * 0.1)):  # 디스플레이 안쪽의 선분인지 판별
            if (y + j) < height_img:
                if (img[y + j, x]) == 255:
                    count = count + 1
        if (count >= 3):  # 흰 픽셀이 3개 이상이면 안쪽으로 간주
            count = 0
            index.append(i)
    for i in reversed(index):  # 디스플레이 안쪽에 해당하는 선분
        del bottom_lines[i]  # 리스트에서 제거
        del bottom_lines[i]

    left_lines, right_lines = np.array(left_lines), np.array(right_lines)  # list를 numpy 배열로 변환
    top_lines, bottom_lines = np.array(top_lines), np.array(bottom_lines)
    if len(left_lines) == 0 or len(right_lines) == 0 or len(top_lines) == 0 or len(bottom_lines) == 0:
        return 0
    left_line = cv2.fitLine(left_lines, cv2.DIST_L2, 0, 0.01, 0.01)
    right_line = cv2.fitLine(right_lines, cv2.DIST_L2, 0, 0.01, 0.01)
    top_line = cv2.fitLine(top_lines, cv2.DIST_L2, 0, 0.01, 0.01)
    bottom_line = cv2.fitLine(bottom_lines, cv2.DIST_L2, 0, 0.01, 0.01)

    d = np.sqrt(left_line[0] * left_line[0] + left_line[1] * left_line[1])
    left_line[0] /= d
    left_line[1] /= d
    t = width_img + height_img
    cv2.line(img_color, ((int)(left_line[2] - left_line[0] * t), (int)(left_line[3] - left_line[1] * t)),
             ((int)(left_line[2] + left_line[0] * t), (int)(left_line[3] + left_line[1] * t)), (255, 0, 0), 3)

    d = np.sqrt(right_line[0] * right_line[0] + right_line[1] * right_line[1])

    right_line[0] /= d
    right_line[1] /= d
    cv2.line(img_color, ((int)(right_line[2] - right_line[0] * t), (int)(right_line[3] - right_line[1] * t)),
             ((int)(right_line[2] + right_line[0] * t), (int)(right_line[3] + right_line[1] * t)), (255, 0, 0), 3)

    d = np.sqrt(top_line[0] * top_line[0] + top_line[1] * top_line[1])
    top_line[0] /= d
    top_line[1] /= d
    cv2.line(img_color, ((int)(top_line[2] - top_line[0] * t), (int)(top_line[3] - top_line[1] * t)),
             ((int)(top_line[2] + top_line[0] * t), (int)(top_line[3] + top_line[1] * t)), (255, 0, 0), 3)

    d = np.sqrt(bottom_line[0] * bottom_line[0] + bottom_line[1] * bottom_line[1])
    bottom_line[0] /= d
    bottom_line[1] /= d
    cv2.line(img_color, ((int)(bottom_line[2] - bottom_line[0] * t), (int)(bottom_line[3] - bottom_line[1] * t)),
             ((int)(bottom_line[2] + bottom_line[0] * t), (int)(bottom_line[3] + bottom_line[1] * t)), (255, 0, 0), 3)

    points = []

    points.append(get_crosspt((int)(left_line[2] - left_line[0] * t), (int)(left_line[3] - left_line[1] * t),
                              (int)(left_line[2] + left_line[0] * t), (int)(left_line[3] + left_line[1] * t),
                              (int)(top_line[2] - top_line[0] * t), (int)(top_line[3] - top_line[1] * t),
                              (int)(top_line[2] + top_line[0] * t), (int)(top_line[3] + top_line[1] * t)))
    points.append(get_crosspt((int)(right_line[2] - right_line[0] * t), (int)(right_line[3] - right_line[1] * t),
                              (int)(right_line[2] + right_line[0] * t), (int)(right_line[3] + right_line[1] * t),
                              (int)(top_line[2] - top_line[0] * t), (int)(top_line[3] - top_line[1] * t),
                              (int)(top_line[2] + top_line[0] * t), (int)(top_line[3] + top_line[1] * t)))
    points.append(get_crosspt((int)(right_line[2] - right_line[0] * t), (int)(right_line[3] - right_line[1] * t),
                              (int)(right_line[2] + right_line[0] * t), (int)(right_line[3] + right_line[1] * t),
                              (int)(bottom_line[2] - bottom_line[0] * t), (int)(bottom_line[3] - bottom_line[1] * t),
                              (int)(bottom_line[2] + bottom_line[0] * t), (int)(bottom_line[3] + bottom_line[1] * t)))
    points.append(get_crosspt((int)(left_line[2] - left_line[0] * t), (int)(left_line[3] - left_line[1] * t),
                              (int)(left_line[2] + left_line[0] * t), (int)(left_line[3] + left_line[1] * t),
                              (int)(bottom_line[2] - bottom_line[0] * t), (int)(bottom_line[3] - bottom_line[1] * t),
                              (int)(bottom_line[2] + bottom_line[0] * t), (int)(bottom_line[3] + bottom_line[1] * t)))
    return points


def transform(img, corners, color):
    pts1 = np.float32([corners[3], corners[0], corners[1], corners[2]])
    width = 410
    height = 810
    pts2 = np.float32([[0, 0], [width - 1, 0], [width - 1, height - 1], [0, height - 1]])
    mtrx = cv2.getPerspectiveTransform(pts1, pts2)
    transformed = cv2.warpPerspective(img, mtrx, (width, height))
    transformed = transformed[5:804, 2:407]
    if (color == 0):
        _, transformed = cv2.threshold(transformed, 128, 255, cv2.THRESH_BINARY)
    return transformed

def expand(img):
    expanded_image = np.pad(img, ((5, 5), (5, 5)), mode='constant', constant_values=255)
    return expanded_image

def line_detection(img_result):
    edges = cv2.Canny(img_result, 100, 200)
    lines = cv2.HoughLines(edges, 1, np.pi / 180, 220)

    vertical_lines = []
    horizontal_lines = []

    if lines is not None:
        for line in lines:
            rho, theta = line[0]
            cos, sin = np.cos(theta), np.sin(theta)
            cx, cy = rho * cos, rho * sin
            x1, y1 = int(cx + 2000 * (-sin)), int(cy + 2000 * cos)
            x2, y2 = int(cx + 2000 * sin), int(cy + 2000 * (-cos))

            cv2.line(img_result, (x1, y1), (x2, y2), (0, 255, 0), 3)

            if (x1 < 1900 and abs(x1 - x2) < 10):
                vertical_lines.append(x1)
            elif (y1 < 1900 and abs(y1 - y2) < 10):
                horizontal_lines.append(y1)
                
    vertical_lines.sort()
    horizontal_lines.sort()

    if lines is not None: 
        return True, vertical_lines, horizontal_lines
    else:
        return False, vertical_lines, horizontal_lines
    


def point_detection(img, min_area, max_area, information):
    params = cv2.SimpleBlobDetector_Params()
    params.minThreshold = 10  # 최소 임계값
    params.maxThreshold = 255  # 최대 임계값
    # params.thresholdStep = 10
    # params.blobColor = 0
    # params.minDistBetweenBlobs = 10
    # params.minRepeatability = 10
    params.filterByCircularity = False
    params.filterByConvexity = False
    params.filterByInertia = False

    params.filterByArea = True
    params.minArea = min_area
    params.maxArea = max_area
    detector = cv2.SimpleBlobDetector_create(params)
    keypoint = detector.detect(img)
    keypoints = []

    for i, blob in enumerate(keypoint):
        redefine = cv2.KeyPoint(x=blob.pt[0], y=blob.pt[1], size=int(blob.size), angle=blob.angle,
                                response=blob.response, octave=blob.octave, class_id=blob.class_id)
        match = False
        for info in information:
            if info[0] - 20 < int(redefine.pt[0]) < info[0] + 20 and info[1] - 20 < int(redefine.pt[1]) < info[1] + 20:
                match = True
                break
        #for j in range(len(vertical_lines)):
            #if (vertical_lines[j] - 7 < int(redefine.pt[0]) < vertical_lines[j] + 7):
                #match = True
                #break

        #for j in range(len(horizontal_lines)):
            #if (horizontal_lines[j] - 7 < int(redefine.pt[1]) < horizontal_lines[j] + 7):
                #match = True
                #break

        #x, y = int(redefine.pt[0]), int(redefine.pt[1])
        #height, width = img_color.shape[:2]
        #if 0 <= x < width and 0 <= y < height:
            #if img_color[y, x, 0] >= 50 or img_color[y, x, 1] >= 50 or img_color[y, x, 2] >= 50:
                #match = True

        if not match:
            keypoints.append(redefine)

    #img_printed = keypoint_print(img, keypoint)
    if keypoints == None:
        return True
    else:
        return False


def keypoint_print(img, blobs):
    for i, blob in enumerate(blobs):
        if (blob.size / 2) ** 2 * np.pi <= 150:
            cv2.circle(img, (int(blob.pt[0]), int(blob.pt[1])), int(blob.size / 2), (0, 0, 255), 5)
        else:
            cv2.circle(img, (int(blob.pt[0]), int(blob.pt[1])), int(blob.size / 2), (0, 0, 255), 5)
    return img


def log_file(dots, patches, vertical_lines, horizontal_lines, start_datetime, finish_datetime):
    if dots or patches or len(vertical_lines) or len(horizontal_lines) > 0:
        dot_count = 0
        patch_count = 0
        line_count = 0

        log_directory = 'C:\\Users\\User\\PycharmProjects\\camera\\log'
        if not os.path.exists(log_directory):
            os.makedirs(log_directory)

        log_filename = start_datetime.strftime("./log/%Y%m%d_%H%M%S.log")
        #log_format = "%(message)s"
        log_format = "%(asctime)s - %(levelname)s - %(message)s"
        logging.basicConfig(filename=log_filename, level=logging.INFO, format=log_format)

        logging.info("시작 시간: %s", start_datetime.strftime("%Y년 %m월 %d일 %H시 %M분 %S초"))
        logging.info("종료 시간: %s", finish_datetime.strftime("%Y년 %m월 %d일 %H시 %M분 %S초"))
        run_time = finish_datetime - start_datetime
        formatted_time = "{:,.0f}".format(run_time.total_seconds() * 1000)
        logging.info("걸린 시간: %s ms", formatted_time)

        ite = 0

        if len(vertical_lines) != 0:
            while (ite < len(vertical_lines)):
                if (ite >= len(vertical_lines) - 1):
                    line_count += 1
                    logging.info("%d번째 선 결함의 위치: x = %d", line_count, vertical_lines[ite])
                    logging.info("%d번째 선 결함의 두께: %d", line_count, 2)
                    ite += 1
                    break
                if ((vertical_lines[ite + 1] - vertical_lines[ite]) > 10):
                    line_count += 1
                    logging.info("%d번째 선 결함의 위치: x = %d", line_count, vertical_lines[ite])
                    logging.info("%d번째 선 결함의 두께: %d", line_count, 2)
                    ite += 1
                else:
                    line_count += 1
                    ite_tmp = ite
                    while ite < (len(vertical_lines) - 1):
                        if (vertical_lines[ite + 1] - vertical_lines[ite]) < 10:
                            ite += 1
                        else:
                            break
                    logging.info("%d번째 선 결함의 위치: x = %d", line_count,
                                 (vertical_lines[ite_tmp] + vertical_lines[ite]) / 2)
                    logging.info("%d번째 선 결함의 두께: %d", line_count, abs(vertical_lines[ite_tmp] - vertical_lines[ite]))
                    ite += 1
        ite = 0

        if len(horizontal_lines) != 0:
            while (ite < len(horizontal_lines)):
                if (ite >= len(horizontal_lines) - 1):
                    line_count += 1
                    logging.info("%d번째 선 결함의 위치: x = %d", line_count, horizontal_lines[ite])
                    logging.info("%d번째 선 결함의 두께: %d", line_count, 2)
                    ite += 1
                    break
                if ((horizontal_lines[ite + 1] - horizontal_lines[ite]) > 10):
                    line_count += 1
                    logging.info("%d번째 선 결함의 위치: x = %d", line_count, horizontal_lines[ite])
                    logging.info("%d번째 선 결함의 두께: %d", line_count, 2)
                    ite += 1
                else:
                    line_count += 1
                    ite_tmp = ite
                    while ite < (len(horizontal_lines) - 1):
                        if (horizontal_lines[ite + 1] - horizontal_lines[ite]) < 10:
                            ite += 1
                        else:
                            break
                    logging.info("%d번째 선 결함의 위치: x = %d", line_count,
                                 (horizontal_lines[ite_tmp] + horizontal_lines[ite]) / 2)
                    logging.info("%d번째 선 결함의 두께: %d", line_count,
                                 abs(horizontal_lines[ite_tmp] - horizontal_lines[ite]))
                    ite += 1

        for i, blob in enumerate(dots):  # blob의 넓이를 구해서 픽셀, 얼룩 결함 판단
            dot_count += 1
            logging.info("%d번째 점 결함의 위치: (%d, %d)", i + 1, blob.pt[0], blob.pt[1])
        for i, blob in enumerate(patches):
            patch_count += 1
            logging.info("%d번째 얼룩 결함의 위치: (%d, %d)", i + 1, blob.pt[0], blob.pt[1])

        logging.info("점 결함 개수: %d개", dot_count)
        logging.info("얼룩 결함 개수: %d개", patch_count)
        logging.info("선 결함 개수: %d개", line_count)
        logging.shutdown()

    return dot_count, patch_count, line_count

def show(img):
    img = cv2.resize(img, (0, 0), fx=0.8, fy=0.8)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

###########################
##추가##

def grab(filename):
    img = cv2.imread(filename, cv2.IMREAD_REDUCED_COLOR_2)

    return img

def morph(img, kernel=(5,5)):
    element_closing = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, kernel)
    img = cv2.morphologyEx(img, cv2.MORPH_CLOSE, element_closing)

    return img

def preprocessing(img_base):
    img_gray = cv2.cvtColor(img_base, cv2.COLOR_BGR2GRAY)
    img_neg = ~img_gray

    img_t = threshold(img_gray)
    corners = deskew(img_t, img_base)
    if corners == 0:
        max_pixel = np.max(img_base, axis=2)
        img_base_2 = img_base.copy()
        img_base_2[:, :, 0] = max_pixel
        img_base_2[:, :, 1] = max_pixel
        img_base_2[:, :, 2] = max_pixel
        img_gray = cv2.cvtColor(img_base_2, cv2.COLOR_BGR2GRAY)
        img_t = threshold(img_gray)
        corners = deskew(img_t, img_base)

    img_transform_color = transform(img_base, corners, 1)
    img_transform = transform(img_neg, corners, 0)

    img_blur = cv2.bilateralFilter(img_transform, 5, 35, 16)
    img_morph = morph(img_blur, (7,7))
    img_morph = ~ img_morph

    return img_transform_color, img_morph

def color_defect(img):
    height, width = img.shape[:2]
    total_pixels = height * width
    count = 0

    for y in range(height):
        for x in range(width):
            b, g, r = img[y, x]

            diff_bg = abs(int(b) - int(g))
            diff_gr = abs(int(g) - int(r))
            diff_br = abs(int(b) - int(r))

            if diff_bg >= 10 or diff_gr >= 10 or diff_br >= 10:
                count += 1

    if count >= 0.1 * total_pixels:
        return True
    else:
        return False

def red_detection(img):
    height, width = img.shape[:2]
    total_pixels = height * width
    count = 0

    if height % 2 != 0:
        height -= 1
    if width % 2 != 0:
        width -= 1

    for y in range(height, 2):
        for x in range(width, 2):
            r = img[y, x, 2]

            if int(r) < 240:
                count += 1

    if count >= 0.1 * total_pixels:
        return False
    else:
        return True

def green_detection(img):
    height, width = img.shape[:2]
    total_pixels = height * width
    count = 0

    if height % 2 != 0:
        height -= 1
    if width % 2 != 0:
        width -= 1

    for y in range(height, 2):
        for x in range(width, 2):
            g = img[y, x, 1]

            if int(g) < 240:
                count += 1

    if count >= 0.1 * total_pixels:
        return False
    else:
        return True

def blue_detection(img):
    height, width = img.shape[:2]
    total_pixels = height * width
    count = 0

    if height % 2 != 0:
        height -= 1
    if width % 2 != 0:
        width -= 1

    for y in range(height, 2):
        for x in range(width, 2):
            b = img[y, x, 0]

            if int(b) < 240:
                count += 1

    if count >= 0.1 * total_pixels:
        return False
    else:
        return True

def stain_detection(img, img_color, connectivity=8):
    num_labels, _, stats, _ = cv2.connectedComponentsWithStats(img, connectivity=connectivity)

    for i in range(1, num_labels):
        x, y, w, h, size = stats[i]

        if (x >= 70 and x <= 100) and (y >= 0 and y <= 20):
            continue
        if size < 30:
            continue
        elif size > 800:
            continue

        density = size / (w * h)

        #얼룩
        if size > 100 and density <= 0.3:
            cv2.rectangle(img_color, (x - 5, y - 5), (x + w + 5, y + h + 5), color=(0, 200, 200), thickness=2)
            return False
        
    return True

def mode_detection(img):
    white = 1
    black = 0
    center_r = img[int(img.shape[0] * 0.45):int(img.shape[0] * 0.55),
                    int(img.shape[1] * 0.45):int(img.shape[1] * 0.55), 2]
    center_g = img[int(img.shape[0] * 0.45):int(img.shape[0] * 0.55),
                    int(img.shape[1] * 0.45):int(img.shape[1] * 0.55), 1]
    center_b = img[int(img.shape[0] * 0.45):int(img.shape[0] * 0.55),
                    int(img.shape[1] * 0.45):int(img.shape[1] * 0.55), 0]

    if int(np.mean(center_r)) > 100 and int(np.mean(center_g)) > 100 and int(np.mean(center_b)) > 100:
        return white
    else:
        return black



