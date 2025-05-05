#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define don_gia_theo_gio 5000  // đơn giá gửi xe mỗi giờ
#define so_luong_cho 50   // So luong cho trong bai giu xe cua moi tang

typedef struct {
    char bien_so_xe[20];
    int phi;
    time_t thoi_gian_vao;
    int tang; 
} phuong_tien;

#define so_luong_cho 100
phuong_tien danh_sach_phuong_tien[so_luong_cho];
int so_luong_xe = 0;
double doanh_thu = 0;

void tong_doanh_thu(double phi);
void luu_doanh_thu();
void mo_doanh_thu();
void cap_nhat_file_bai_do();

void doc_file_bai_do() {
    FILE *tep = fopen("parking_data.txt", "r");
    if (tep == NULL) {
        tep = fopen("parking_data.txt", "w"); // Tạo file nếu chưa có
        if (tep) fclose(tep);
        return;
    }

    phuong_tien tam;
    int nam, thang, ngay, gio, phut, giay;
    so_luong_xe = 0;

    while (fscanf(tep, "%s %d %d-%d-%d %d:%d:%d %d",
                  tam.bien_so_xe, &tam.phi,
                  &nam, &thang, &ngay, &gio, &phut, &giay, &tam.tang) == 9) {
        struct tm thoi_gian = {0};
        thoi_gian.tm_year = nam - 1900;
        thoi_gian.tm_mon = thang - 1;
        thoi_gian.tm_mday = ngay;
        thoi_gian.tm_hour = gio;
        thoi_gian.tm_min = phut;
        thoi_gian.tm_sec = giay;

        tam.thoi_gian_vao = mktime(&thoi_gian);

        if (so_luong_xe < so_luong_cho) {
            danh_sach_phuong_tien[so_luong_xe++] = tam;
        }
    }
    fclose(tep);
}

void cap_nhat_file_bai_do() {
    FILE *tep = fopen("parking_data.txt", "w");
    if (!tep) return;
    for (int i = 0; i < so_luong_xe; i++) {
        char chuoi_thoi_gian[26];
        strftime(chuoi_thoi_gian, sizeof(chuoi_thoi_gian), "%Y-%m-%d %H:%M:%S", localtime(&danh_sach_phuong_tien[i].thoi_gian_vao));
        fprintf(tep, "%s %d %s %d\n", danh_sach_phuong_tien[i].bien_so_xe, danh_sach_phuong_tien[i].phi, chuoi_thoi_gian, danh_sach_phuong_tien[i].tang);
    }
    fclose(tep);
}

int kiem_tra_con_cho_trong() {
    return so_luong_xe < so_luong_cho;
}

void luu_phuong_tien(phuong_tien* xe_moi) {
    FILE *tep = fopen("parking_data.txt", "a");
    if (tep == NULL) {
        printf("Khong the mo file de ghi!\n");
        return;
    }

    char chuoi_thoi_gian[26];
    strftime(chuoi_thoi_gian, sizeof(chuoi_thoi_gian), "%Y-%m-%d %H:%M:%S", localtime(&xe_moi->thoi_gian_vao));
    fprintf(tep, "%s %d %s %d\n", xe_moi->bien_so_xe, xe_moi->phi, chuoi_thoi_gian, xe_moi->tang);
    fclose(tep);
}

void kiem_tra_bien_so_xe(char *bien_so) {
    while (1) {
        int dem = 0;
        if (strlen(bien_so) == 10) {
            if (isdigit(bien_so[0]) && isdigit(bien_so[1])) dem++;
            if (isalpha(bien_so[2])) dem++;
            if (bien_so[3] == '-') dem++;
            if (isdigit(bien_so[4]) && isdigit(bien_so[5]) && isdigit(bien_so[6])) dem++;
            if (bien_so[7] == '.') dem++;
            if (isdigit(bien_so[8]) && isdigit(bien_so[9])) dem++;
        }

        if (dem == 6) {
            return;
        } else {
            printf("Bien so khong dung dinh dang! Nhap lai (dinh dang: XXA-XXX.XX): ");
            scanf("%s", bien_so);
        }
    }
}

void ghi_nhat_ky(const char *bien_so, const char *hanh_dong) {
    FILE *log = fopen("log.txt", "a");
    if (!log) {
        printf("Khong the mo log.txt de ghi!\n");
        return;
    }

    time_t bay_gio = time(NULL);
    struct tm *thoi_gian = localtime(&bay_gio);
    char chuoi_thoi_gian[30];
    strftime(chuoi_thoi_gian, sizeof(chuoi_thoi_gian), "%Y-%m-%d %H:%M:%S", thoi_gian);
    fprintf(log, "%s %s %s\n", bien_so, hanh_dong, chuoi_thoi_gian);
    fclose(log);
}

void them_phuong_tien() {
    if (!kiem_tra_con_cho_trong()) {
        printf("Bai do xe da day!\n");
        return;
    }

    phuong_tien xe_moi;
    printf("Chu y: Dinh dang bien so 2 chu so + 1 chu cai + '-' + 3 chu so + '.' + 2 chu so.\n");
    printf("Nhap bien so xe: ");
    fgets(xe_moi.bien_so_xe, sizeof(xe_moi.bien_so_xe), stdin);
    xe_moi.bien_so_xe[strcspn(xe_moi.bien_so_xe, "\n")] = '\0';
    kiem_tra_bien_so_xe(xe_moi.bien_so_xe);

    printf("Nhap so tang (1/2/3/4): ");
    scanf("%d", &xe_moi.tang);
    getchar();

    for (int i = 0; i < so_luong_xe; i++) {
        if (strcmp(xe_moi.bien_so_xe, danh_sach_phuong_tien[i].bien_so_xe) == 0) {
            printf("Bien so da ton tai!\n");
            return;
        }
    }

    xe_moi.thoi_gian_vao = time(NULL);
    xe_moi.phi = 0;
    danh_sach_phuong_tien[so_luong_xe++] = xe_moi;
    luu_phuong_tien(&xe_moi);

    printf("Them xe thanh cong!\n");
    ghi_nhat_ky(xe_moi.bien_so_xe, "in");
}

void hien_thi_danh_sach_phuong_tien() {
    if (so_luong_xe == 0) {
        printf("Bai xe hien tai rong!\n");
        return;
    }

    printf("%-5s %-20s %-25s %-10s %-6s\n", "STT", "Bien so xe", "Thoi gian vao bai", "Phi", "Tang");
    for (int i = 0; i < so_luong_xe; i++) {
        char chuoi_thoi_gian[26];
        strftime(chuoi_thoi_gian, sizeof(chuoi_thoi_gian), "%Y-%m-%d %H:%M:%S", localtime(&danh_sach_phuong_tien[i].thoi_gian_vao));
        printf("%-5d %-20s %-25s %-10d %-6d\n", 
               i + 1, 
               danh_sach_phuong_tien[i].bien_so_xe, 
               chuoi_thoi_gian, 
               danh_sach_phuong_tien[i].phi, 
               danh_sach_phuong_tien[i].tang);
    }
}

phuong_tien* tim_phuong_tien(const char *bien_so) {
    for (int i = 0; i < so_luong_xe; i++) {
        if (strcmp(danh_sach_phuong_tien[i].bien_so_xe, bien_so) == 0) {
            return &danh_sach_phuong_tien[i];
        }
    }
    return NULL;
}

void xoa_phuong_tien(const char *bien_so) {
    phuong_tien *xe = tim_phuong_tien(bien_so);
    if (xe == NULL) {
        printf("Khong tim thay xe!\n");
        return;
    }

    time_t bay_gio = time(NULL);
    double thoi_gian_trong_bai = difftime(bay_gio, xe->thoi_gian_vao);
    int gio = (int)(thoi_gian_trong_bai / 3600);
    int phut = ((int)thoi_gian_trong_bai % 3600) / 60;
    int giay = (int)thoi_gian_trong_bai % 60;

    int tong_gio = (thoi_gian_trong_bai + 3599) / 3600;
    xe->phi = tong_gio * don_gia_theo_gio;

    printf("Thoi gian gui xe %s: %02d:%02d:%02d (hh:mm:ss)\n", bien_so, gio, phut, giay);
    printf("Tong phi: %d VND\n", xe->phi);
    tong_doanh_thu(xe->phi);
    ghi_nhat_ky(bien_so, "out");

    for (int i = 0; i < so_luong_xe; i++) {
        if (strcmp(danh_sach_phuong_tien[i].bien_so_xe, bien_so) == 0) {
            for (int j = i; j < so_luong_xe - 1; j++)
                danh_sach_phuong_tien[j] = danh_sach_phuong_tien[j + 1];
            so_luong_xe--;
            break;
        }
    }
    printf("Xe da roi bai thanh cong.\n");
    cap_nhat_file_bai_do();
}

void hien_thi_tong_tien() {
    printf("\nTong tien thu duoc: %.0f VND\n", doanh_thu);
}

void thong_ke_xe_ra_vao() {
    FILE *tep = fopen("log.txt", "r");
    if (tep == NULL) {
        printf("Khong the mo file log.txt!\n");
        return;
    }

    int tong_vao = 0;
    int tong_ra = 0;
    char bien_so[20], hanh_dong[10];
    char chuoi_thoi_gian[30];

    while (fscanf(tep, "%s %s %[^]", bien_so, hanh_dong, chuoi_thoi_gian) == 3) {
        if (strcmp(hanh_dong, "in") == 0) tong_vao++;
        else if (strcmp(hanh_dong, "out") == 0) tong_ra++;
    }

    fclose(tep);
    printf("Tong so xe vao: %d\n", tong_vao);
    printf("Tong so xe ra : %d\n", tong_ra);
}

void hien_thi_thong_ke_theo_tang() {
    int xe_tang_1 = 0, xe_tang_2 = 0, xe_tang_3 = 0, xe_tang_4 = 0;

    for (int i = 0; i < so_luong_xe; i++) {
        switch (danh_sach_phuong_tien[i].tang) {
            case 1: xe_tang_1++; break;
            case 2: xe_tang_2++; break;
            case 3: xe_tang_3++; break;
            case 4: xe_tang_4++; break;
        }
    }

    printf("\n--- Thong ke bai do xe theo tang ---\n");
    printf("Tang 1: %d xe | Con lai: %d cho\n", xe_tang_1, so_luong_cho - xe_tang_1);
    printf("Tang 2: %d xe | Con lai: %d cho\n", xe_tang_2, so_luong_cho - xe_tang_2);
    printf("Tang 3: %d xe | Con lai: %d cho\n", xe_tang_3, so_luong_cho - xe_tang_3);
    printf("Tang 4: %d xe | Con lai: %d cho\n", xe_tang_4, so_luong_cho - xe_tang_4);
}

int main() {
    mo_doanh_thu();
    doc_file_bai_do();
    int lua_chon;
    char bien_so_xe[20];

    while (1) {
        printf("\n=== Quan ly bai do xe ===\n");
        printf("1. Them xe vao bai\n");
        printf("2. Xe roi bai\n");
        printf("3. Xem danh sach xe\n");
        printf("4. Tong doanh thu\n");
        printf("5. Thong ke xe ra/vao\n");
        printf("6. Thoat\n");
        printf("7. Thong ke theo tang\n");
        printf("Chon chuc nang: ");
        scanf("%d", &lua_chon);
        getchar();

        switch (lua_chon) {
            case 1:
                them_phuong_tien();
                break;
            case 2:
                printf("Nhap bien so xe: ");
                scanf("%s", bien_so_xe);
                xoa_phuong_tien(bien_so_xe);
                break;
            case 3:
                hien_thi_danh_sach_phuong_tien();
                break;
            case 4:
                hien_thi_tong_tien();
                break;
            case 5:
                thong_ke_xe_ra_vao();
                break;
            case 6:
                return 0;
            case 7:
                hien_thi_thong_ke_theo_tang();
                break;
            default:
                printf("Lua chon khong hop le!\n");
        }
    }
}
