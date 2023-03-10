//
// Created by admin on 2021/11/16.
//
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"

#include "Draw.h"
#include <jni.h>
#include <android/log.h>
#include <ctime>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/system_properties.h>
#include <string>
#include <Logger.h>
#include <sys/mman.h>
#include <imgui.h>
#include <cstring>
#include <string>
#include <GLES3/gl3.h>
#include <ctgmath>
#include <imgui_internal.h>
#include <dirent.h>
#include <stb_image.h>
#include <imgui_expand.h>
#include <files.h>
#include <base64/base64.h>


using namespace std;
static TextureInfo textureInfo;
extern int glWidth, glHeight;
extern int screenWidth;
extern bool openAccumulation;
extern float px, py;
extern TextureInfo back1;
extern TextureInfo back2;

TextureInfo back1;
TextureInfo back2;
ImFont* font_windows;
ImFont* font_draw;

const char* strstri(const char* str, const char* subStr) {
    int len = strlen(subStr);
    if (len == 0) {
        return nullptr;
    }

    while(*str) {
        if (strncasecmp(str, subStr,len) == 0) {
            return str;
        }
        ++str;
    }
    return nullptr;
}

TextureInfo createTexture(const string &ImagePath) {
    int w, h, n;
    stbi_uc *data = stbi_load(ImagePath.c_str(), &w, &h, &n, 0);
    GLuint texture;
    glGenTextures(1, &texture);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // if (n == 3) {
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    // } else {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //}
    stbi_image_free(data);
    textureInfo.textureId = (GLuint *) texture;
    textureInfo.width = w;
    textureInfo.height = h;
    return textureInfo;
}

static bool isVisible(float z) {
    return z != 0;
}


static void DrawRadar(float Angle, float x, float y, ImVec4 color) {
    float w = 45.0f / 2 / 1.5f;
    float b = 22.5f;
    if (Angle <= b || Angle >= 360 - b) {
        CustomImDrawList::drawLine(x - w, y + w, x, y - w, color, 1);
        CustomImDrawList::drawLine(x, y - w, x + w, y + w, color, 1);
    } else if (Angle >= 90 - b && Angle <= 90 + b) {
        CustomImDrawList::drawLine(x - w, y - w, x + w, y, color, 1);
        CustomImDrawList::drawLine(x + w, y, x - w, y + w, color, 1);
    } else if (Angle >= 180 - b && Angle <= 180 + b) {
        CustomImDrawList::drawLine(x - w, y - w, x, y + w, color, 1);
        CustomImDrawList::drawLine(x, y + w, x + w, y - w, color, 1);
    } else if (Angle >= 270 - b && Angle <= 270 + b) {
        CustomImDrawList::drawLine(x + w, y - w, x - w, y, color, 1);
        CustomImDrawList::drawLine(x - w, y, x + w, y + w, color, 1);
    } else if (Angle >= 45 - b && Angle <= 45 + b) {
        CustomImDrawList::drawLine(x + w, y - w, x - w, y, color, 1);
        CustomImDrawList::drawLine(x + w, y - w, x, y + w, color, 1);
    } else if (Angle >= 135 - b && Angle <= 135 + b) {
        CustomImDrawList::drawLine(x + w, y + w, x - w, y, color, 1);
        CustomImDrawList::drawLine(x + w, y + w, x, y - w, color, 1);
    } else if (Angle >= 225 - b && Angle <= 225 + b) {
        CustomImDrawList::drawLine(x - w, y + w, x, y - w, color, 1);
        CustomImDrawList::drawLine(x - w, y + w, x + w, y, color, 1);
    } else if (Angle >= 315 - b && Angle <= 315 + b) {
        CustomImDrawList::drawLine(x - w, y - w, x + w, y, color, 1);
        CustomImDrawList::drawLine(x - w, y - w, x, y + w, color, 1);
    }

}




string GetGrenadeInfo(const char *gname ,ImVec4 *color) {
    string name;
    if (strstr(gname, "ProjGrenade_BP_C") != nullptr)//??????
    {
        name = "????????????";
        *color = ImVec4{255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 255.f / 255.f};
        return name;
    }
    if (strstr(gname, "ProjSmoke_BP_C") != nullptr) {
        name = "??????????????????";
        *color = ImVec4{255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 255.f / 255.f};
        return name;
    }
    if (strstr(gname, "ProjBurn_BP_C") != nullptr) {
        name = "???????????????";
        *color = ImVec4{255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 255.f / 255.f};
        return name;
    }
    if (strstr(gname, "BP_Grenade_Pan_C") != nullptr) {
        name = "???????????????";
        *color = ImVec4{255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 255.f / 255.f};
        return name;
    }
    return "";
}


int getFontHeight() {
    ImVec2 text_size = font_draw->CalcTextSizeA(font_draw->FontSize, FLT_MAX, -1.0f, "Hello World");
    return (int)text_size.y;
}

string getBoxName(int id) {
    if (id == 601006) return "[??????]?????????";
    if (id == 601005) return "[??????]?????????";
    if (id == 601004) return "[??????]??????";
    if (id == 601001) return "[??????]??????";
    if (id == 601002) return "[??????]?????????";
    if (id == 601003) return "[??????]?????????";


    if (id == 503001) return "[??????]???????????????";
    if (id == 503002) return "[??????]???????????????";
    if (id == 503003) return "[??????]???????????????";
    if (id == 502001) return "[??????]????????????";
    if (id == 502002) return "[??????]????????????";
    if (id == 502003) return "[??????]????????????";
    if (id == 501001) return "[??????]?????????";
    if (id == 501002) return "[??????]?????????";
    if (id == 501006) return "[??????]?????????";


    if (id == 107001) return "[??????]?????????";
    if (id == 103003) return "[??????]AWM";
    if (id == 103001) return "[??????]Kar98K";
    if (id == 105002) return "[??????]DP28";
    if (id == 103002) return "[??????]M24";
    if (id == 101005) return "[??????]??????";
    if (id == 101001) return "[??????]AKM";
    if (id == 106005) return "[??????]R45";
    if (id == 101009) return "[??????]MK47";
    if (id == 101006) return "[??????]AUG";
    if (id == 103006) return "[??????]Mini14";
    if (id == 101002) return "[??????]M16A4";
    if (id == 101003) return "[??????]SCAR";
    if (id == 102001) return "[??????]UZI";
    if (id == 102004) return "[??????]??????????????????";
    if (id == 102003) return "[??????]?????????";
    if (id == 102002) return "[??????]UMP9";
    if (id == 103005) return "[??????]VSS????????????";
    if (id == 103008) return "[??????]Win94";
    if (id == 103009) return "[??????]SLR";
    if (id == 105001) return "[??????]QBU";
    if (id == 101007) return "[??????]QBZ";
    //if(id==105001,return "[??????]?????????";
    if (id == 106003) return "[??????]R1895";
    if (id == 101004) return "[??????]M416";
    if (id == 106006) return "[??????]???????????????";
    if (id == 104003) return "[??????]S12K";
    if (id == 104002) return "[??????]S1897";
    if (id == 104001) return "[??????]????????????";
    if (id == 102105) return "[??????]P90?????????";
    if (id == 103012) return "[??????]AMR?????????";

    if (id == 306001) return "[??????]?????????";
    if (id == 302001) return "[??????]762MM";
    if (id == 303001) return "[??????]5.56MM";
    if (id == 301001) return "[??????]9MM";
    if (id == 304001) return "[??????]12??????";
    if (id == 305001) return "[??????]45??????";
    if (id == 301002) return "[??????]5.7MM";
    if (id == 306002) return "[??????]50??????";
    if (id == 308001) return "[??????]?????????";
    if (id == 307001) return "[??????]??????";


    if (id == 203001) return "[??????]??????";
    if (id == 203002) return "[??????]??????";
    if (id == 203003) return "[??????]2??????";
    if (id == 203014) return "[??????]3??????";
    if (id == 203004) return "[??????]4??????";
    if (id == 203015) return "[??????]6??????";
    if (id == 203005) return "[??????]8??????";


    if (id == 204014) return "[??????]?????????";
    if (id == 205004) return "[??????]??????";
    if (id == 204010) return "[??????]?????????";
    if (id == 202007) return "[??????]???????????????";
    if (id == 202004) return "[??????]????????????";
    if (id == 202005) return "[??????]????????????";
    if (id == 205001) return "[??????]????????????";
    if (id == 205003) return "[??????]????????????";
    if (id == 205002) return "[??????]????????????";
    //if(id==205001,return "[??????]??????";
    if (id == 201003) return "[??????]??????????????????";
    if (id == 201005) return "[??????]??????????????????";
    if (id == 201007) return "[??????]??????????????????";
    if (id == 201011) return "[??????]???????????????";
    if (id == 201009) return "[??????]???????????????";
    if (id == 201010) return "[??????]???????????????";
    if (id == 201006) return "[??????]??????????????????";
    if (id == 201004) return "[??????]??????????????????";
    if (id == 201002) return "[??????]??????????????????";
    if (id == 204009) return "[??????]??????????????????";
    if (id == 204007) return "[??????]???????????????";
    if (id == 204008) return "[??????]???????????????";
    if (id == 204013) return "[??????]??????????????????";
    if (id == 204011) return "[??????]????????????";
    if (id == 204012) return "[??????]????????????";
    if (id == 204006) return "[??????]?????????????????????";
    if (id == 204004) return "[??????]???????????????";
    if (id == 204005) return "[??????]???????????????";
    if (id == 204003) return "[??????]????????????";
    if (id == 204002) return "[??????]????????????";
    if (id == 204001) return "[??????]????????????";


    if (id == 602003) return "[??????]?????????";
    if (id == 602002) return "[??????]?????????";
    if (id == 602001) return "[??????]??????";
    if (id == 603001) return "[??????]??????";
    if (id == 403990) return "[??????]?????????";
    if (id == 403187) return "[??????]?????????";
    return "Error";
}

void drawRadar(float dis, float RotationAngleX,float RotationAngleY, bool isAI, bool isDie, float MapX, float MapY, float RadarLocationX,float RadarLocationY, float angle) {
    // ????????????
    if (!showRadar) {
        return;
    }
    ImVec4 LC = {0, 0, 0, 0};
    if (isDie) {
        LC = fallen_color;
    }  else if(isAI) {
        LC = ai_color;
    } else {
        LC = radar_color;
    }

    if (dis > 0 && dis < 450) {

        float Offset_Maps[2] = {0, 0};

        Offset_Maps[0] = MapX;
        Offset_Maps[1] = MapY;

        if (radarType == 1) {
            float proportion = radarSize / 100;
            float MapSize = round(265 * proportion) / 2;

            float RadarLocation_X = RadarLocationX / (62.5f * 2.5f / proportion);
            float RadarLocation_Y = RadarLocationY / (62.5f * 2.5f / proportion);

            if (showRadarBox) {
                float my_height = MapSize;
                float my_width = MapSize;
                float off = MapSize / 2;

                CustomImDrawList::drawLine(0 + Offset_Maps[0] - off, 0 + Offset_Maps[1] - off,my_width + Offset_Maps[0] - off,my_height + Offset_Maps[1] - off, Color.Yellow, 1);

                CustomImDrawList::drawLine(my_width + Offset_Maps[0] - off, 0 + Offset_Maps[1] - off,0 + Offset_Maps[0] - off,my_height + Offset_Maps[1] - off, Color.Yellow, 1);

                CustomImDrawList::drawRect(0 + Offset_Maps[0] - off, 0 + Offset_Maps[1] - off,my_width + Offset_Maps[0] - off,my_height + Offset_Maps[1] - off, Color.Black, 1);

                CustomImDrawList::drawRect(my_width / 2 - 6 + Offset_Maps[0] - off,my_height / 2 - 6 + Offset_Maps[1] - off,my_width / 2 + 6 + Offset_Maps[0] - off,my_height / 2 + 6 + Offset_Maps[1] - off,Color.Black, 1);
            }

            if (RadarLocation_X < (-MapSize / 2.0f + 45.0f / 2.0f) || RadarLocation_Y < (-MapSize / 2.0f + 45.0f / 2.0f) || RadarLocation_X > (MapSize / 2.0f - 45.0f / 2.0f) || RadarLocation_Y > (MapSize / 2.0f - 45.0f / 2.0f)) {
                float x1 = abs(RadarLocation_X);
                float y1 = abs(RadarLocation_Y);
                float z1 = max(x1, y1) / ((MapSize / 2) - (45.0f / 2.0f));
                RadarLocation_X = RadarLocation_X / z1;
                RadarLocation_Y = RadarLocation_Y / z1;
            }
            DrawRadar(angle, Offset_Maps[0] + RadarLocation_X, Offset_Maps[1] + RadarLocation_Y,LC);
            CustomImDrawList::drawRectFilled(Offset_Maps[0] + RadarLocation_X - 7.5f,Offset_Maps[1] + RadarLocation_Y - 7.5f,Offset_Maps[0] + RadarLocation_X + 7.5f ,Offset_Maps[1] + RadarLocation_Y + 7.5f, LC);
            if (showRadarDis) {
                string temp;
                if (dis > 9 && dis < 100) {
                    temp = " " + to_string((int) ceil(dis));
                } else if (dis < 10) {
                    temp = "  " + to_string((int) ceil(dis));
                } else {
                    temp = to_string((int) ceil(dis));
                }
                //DrawText(temp,Offset_Maps[0] + RadarLocation_X - 20 ,Offset_Maps[1] + RadarLocation_Y - 17, info_color);
                CustomImDrawList::drawText(temp.c_str(),Offset_Maps[0] + RadarLocation_X - 13 ,Offset_Maps[1] + RadarLocation_Y - 10, info_color);
            }
        } else {
            float ox = 0;//ImGui::GetWindowPos().x;
            float oy = 0;//ImGui::GetWindowPos().y;
            CustomImDrawList::drawCircle(Offset_Maps[0] + ox, Offset_Maps[1] + oy, 140, Color.White, 0, 3);
            CustomImDrawList::drawCircle(Offset_Maps[0]+ ox, Offset_Maps[1] + oy, 30, Color.White, 0, 3);
            if (showRadarDis) {
                string temp = to_string((int) dis) + "m";
                CustomImDrawList::drawText(temp.c_str(), RotationAngleX + 40 + Offset_Maps[0]+ ox,RotationAngleY + Offset_Maps[1] + oy, info_color);
            }
            CustomImDrawList::drawCircleFilled(RotationAngleX + Offset_Maps[0]+ ox, RotationAngleY + Offset_Maps[1]+oy,10, LC, 0);
            CustomImDrawList::drawCircleFilled(Offset_Maps[0]+ ox, Offset_Maps[1]+oy, 10, Color.Yellow, 0);

        }
    }
}

void initDraw() {
    //????????????
    if (openAccumulation && showCrosshair) {
        int radarLeft = px, radarTop = py;
        CustomImDrawList::drawLine(radarLeft - 40 + 15, radarTop, radarLeft + 40 - 15, radarTop, Color.White, 1);
        CustomImDrawList::drawLine(radarLeft, radarTop - 40 + 15, radarLeft, radarTop + 40 - 15, Color.White, 1);
    }

    if (isProjSomokeRange && isProjSomoke) {
        CustomImDrawList::drawRectFilled(px - projSomoke, py - projSomoke, px + projSomoke,py + projSomoke, Color.Red_);
    }
}

void DrawNum(int PlayerCount) {
    if (!showNum) return;
    char countTemp[12];
    sprintf(countTemp, "%d", PlayerCount);
    if (PlayerCount == 0) {
        CustomImDrawList::drawImage(px - 95, py / 10 + 10, 180, 40, back2.textureId);
        CustomImDrawList::drawText(px - 18, py / 10 + 18, Color.White, "??????");
    } else {
        CustomImDrawList::drawImage(px - 95, py / 10 + 10 , 180, 40, back1.textureId);
        CustomImDrawList::drawText(px - 10, py / 10 + 18, Color.White, countTemp);
    }
}

void drawBoneLine(Vector3A start, Vector3A end, bool Die, bool isAi) {
    ImVec4 BC = {0, 0, 0, 0};
    if ((isVisible(start.Z) || isVisible(end.Z)) || !isVisibility) {
        if (Die) {
            BC = fallen_color;
        } else if (isAi) {
            BC = ai_color;
        } else {
            BC = bone_color;
        }
    }else {
        if (Die) {
            BC = fallen_color;
        } else {
            BC = visibility_color;
        }
    }
    CustomImDrawList::drawLine(start.X, start.Y, end.X, end.Y, BC, boneWidth);
}

void DrawBox(PlayerData obj, bool vis, bool Die, bool isAi) {
    if (!showBox) {
        return;
    }
    ImVec4 BC = {0, 0, 0, 0};
    if (vis || !isVisibility) {
        if (Die) {
            BC = fallen_color;
        } else if (isAi) {
            BC = ai_color;
        } else {
            BC = box_color;
        }
    }else {
        if (Die) {
            BC = fallen_color;
        } else {
            BC = visibility_color;
        }
    }
    BoneData mBoneData = obj.mBoneData;
    if (mBoneData.Left_Ankle.Y > mBoneData.Right_Ankle.Y) {
        CustomImDrawList::drawRect(mBoneData.Pelvis.X - obj.Location.w / 2 , mBoneData.Head.Y - obj.HeadSize - 5, mBoneData.Pelvis.X + obj.Location.w / 2, mBoneData.Left_Ankle.Y + 5, BC, boxWidth);
    } else {
        CustomImDrawList::drawRect(mBoneData.Pelvis.X - obj.Location.w / 2, mBoneData.Head.Y  - obj.HeadSize - 5, mBoneData.Pelvis.X + obj.Location.w / 2, mBoneData.Right_Ankle.Y + 5, BC, boxWidth);
    }
}

void DrawBone(PlayerData obj,bool isDie) {
    if (!showBone) {
        return;
    }
    //????????????
    BoneData bones = obj.mBoneData;
    //???
    ImVec4 head_color = {0, 0, 0, 0};
    if ((isVisible(bones.Head.Z) || isVisible(bones.Head.Z)) || !isVisibility) {
        if (isDie) {
            head_color = fallen_color;
        } else if (obj.isAI) {
            head_color = ai_color;
        } else {
            head_color = bone_color;
        }
    }else {
        if (isDie) {
            head_color = fallen_color;
        } else {
            head_color = visibility_color;
        }
    }
    CustomImDrawList::drawCircle(bones.Head.X, bones.Head.Y, obj.HeadSize, head_color, 0, boneWidth);
    drawBoneLine(bones.vNeck, bones.Chest, isDie, obj.isAI);
    drawBoneLine(bones.Chest, bones.Pelvis, isDie, obj.isAI);
    drawBoneLine(bones.vNeck, bones.Left_Shoulder, isDie, obj.isAI);
    drawBoneLine(bones.vNeck, bones.Right_Shoulder, isDie, obj.isAI);
    drawBoneLine(bones.Left_Shoulder, bones.Left_Elbow, isDie, obj.isAI);
    drawBoneLine(bones.Left_Elbow, bones.Left_Wrist, isDie, obj.isAI);
    drawBoneLine(bones.Right_Shoulder, bones.Right_Elbow, isDie, obj.isAI);
    drawBoneLine(bones.Right_Elbow, bones.Right_Wrist, isDie, obj.isAI);
    drawBoneLine(bones.Pelvis, bones.Left_Thigh, isDie, obj.isAI);
    drawBoneLine(bones.Pelvis, bones.Right_Thigh, isDie, obj.isAI);
    drawBoneLine(bones.Left_Thigh, bones.Left_Knee, isDie, obj.isAI);
    drawBoneLine(bones.Left_Knee, bones.Left_Ankle, isDie, obj.isAI);
    drawBoneLine(bones.Right_Thigh, bones.Right_Knee, isDie, obj.isAI);
    drawBoneLine(bones.Right_Knee, bones.Right_Ankle, isDie, obj.isAI);

}

float get2dDistance(float x, float y, float x1, float y1);

void DrawInfo(float entityHealth, int TeamID, char * name, float d, bool isRat, bool isAi,bool vis, bool Die, PlayerData obj) {

    float width = glWidth, height = glHeight;
    //??????
    string BottomText;
    if (showHealth) {
        BottomText.append(to_string((int)entityHealth)).append("HP ");
    }

    if (showDis) {
        BottomText.append(to_string((int)d)).append("M ");
    }

    if (showWeapon) {
        BottomText.append(getWeapon(obj.Weapon.id));
        if (isKernel && obj.Weapon.TotalBullet > 1) {
            BottomText.append("(").append(to_string(obj.Weapon.CurrentBullet)).append("/").append(to_string(obj.Weapon.TotalBullet)).append(")");
        }
    }

    //??????
    BoneData mBoneData = obj.mBoneData;
    string TopText;

    if (showTeam) {
        TopText.append(to_string(TeamID)).append("??? ");
    }

    if (showInfo) {
        if (isAi || isRat) {
            TopText.append(isAi ? "??????" : "??????");
        }
    }

    if (showName) {
        if (!isAi && !isRat) {
            TopText.append(name);
        }
    }

    if (TopText.length() != 0) {
        CustomImDrawList::drawText(TopText.c_str(),mBoneData.Pelvis.X, mBoneData.Head.Y - 28 - obj.HeadSize, info_color,font_draw,true);
    }

    if (BottomText.length() != 0) {
        if (mBoneData.Left_Ankle.Y > mBoneData.Right_Ankle.Y) {
            CustomImDrawList::drawText(BottomText.c_str(), mBoneData.Pelvis.X, mBoneData.Left_Ankle.Y + 6 , info_color,font_draw,true);
        } else {
            CustomImDrawList::drawText(BottomText.c_str(), mBoneData.Pelvis.X , mBoneData.Right_Ankle.Y + 6 , info_color,font_draw,true);
        }
    }

    //????????????
    if (showLine) {
        ImVec4 BC = {0, 0, 0, 0};
        if (vis || !isVisibility) {
            if (Die) {
                BC = fallen_color;
            } else if (isAi) {
                BC = ai_color;
            } else {
                BC = line_color;
            }
        }else {
            if (Die) {
                BC = fallen_color;
            } else {
                BC = visibility_color;
            }
        }
        CustomImDrawList::drawLine(px, py / 10 + 50, mBoneData.Head.X, mBoneData.Head.Y - obj.HeadSize - 22, BC, lineWidth);
    }

    //??????
    DrawBox(obj, vis, Die, isAi);
    //??????
    DrawBone(obj, Die);
}

void InitTexture() {
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsLight();

    //Imgui????????????
    size_t length = (files::fontDataBase64.length( ) + 1) / 4 * 3;
    unsigned char *fontData = base64_decode((unsigned char *) files::fontDataBase64.c_str( ));
    font_windows = io.Fonts->AddFontFromMemoryTTF((void *) fontData, length, 24.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull( ));
    font_draw = io.Fonts->AddFontFromMemoryTTF((void *) fontData, length, 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull( ));

    //????????????
    length = (files::countDataBase64.length() + 1) / 4 * 3;
    unsigned char * data = base64_decode((unsigned char *) files::countDataBase64.c_str( ));
    back1 = ImGui::loadTextureFromMemory(data, length);
    free(data);

    //????????????
    length = (files::safeDataBase64.length() + 1) / 4 * 3;
    unsigned char * data2 = base64_decode((unsigned char *) files::safeDataBase64.c_str( ));
    back2 = ImGui::loadTextureFromMemory(data2, length);
    free(data2);
}

string GetItemInfo(const char *gname, ImVec4 *color) {
    string name;
    if (show556) {
        if (strcmp(gname, "BP_Ammo_556mm_Pickup_C") == 0) {
            name = "5.56MM";
            *color = color_bullet;
            return name;
        }
        if (strcmp(gname, "BP_Ammo_762mm_Pickup_C") == 0) {
            name = "7.62MM";
            *color = color_bullet;
            return name;
        }
        if (strstr(gname, "Ammo_45AC") != nullptr) {
            name = "45ACP";
            *color = color_bullet;
            return name;
        }
        if (strstr(gname, "Ammo_9mm") != nullptr) {
            name = "9mm";
            *color = color_bullet;
            return name;
        }
        if (strstr(gname, "Ammo_12Guage") != nullptr) {
            name = "12 Guage";
            *color = color_bullet;
            return name;
        }
        if (strstr(gname, "Ammo_300Magnum") != nullptr) {
            name = "300?????????";
            *color = color_bullet;
            return name;
        }
        if (strstr(gname, "Ammo_Bolt") != nullptr) {
            name = "??????";
            *color = color_bullet;
            return name;
        }
        if (strstr(gname, "BP_Ammo_50BMG_Pickup_C") != nullptr) {
            name = "AWR??????";
            *color = color_bullet;
            return name;
        }
    }

    //?????????
    if (showSandan) {
        if (strstr(gname, "S12K") != nullptr) {
            name = "S12K";
            *color = color_shot;
            return name;
        }
        if (strstr(gname, "DBS") != nullptr) {
            name = "DBS?????????";
            *color = color_shot;
            return name;
        }
        if (strstr(gname, "S686") != nullptr) {
            name = "S686";
            *color = color_shot;
            return name;
        }
        if (strstr(gname, "S1897") != nullptr) {
            name = "S1897";
            *color = color_shot;
            return name;
        }
        if (strstr(gname, "DP12") != nullptr) {
            name = "DBS";
            *color = color_shot;
            return name;
        }
        if (strstr(gname, "BP_ShotGun_SPAS-12_Wrapper_C") != nullptr) {
            name = "SPAS_12";
            *color = color_shot;
            return name;
        }
    }
    //?????????
    if (showTouzhi) {
        if (strstr(gname, "Grenade_Stun") != nullptr) {
            name = "?????????";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "Grenade_Shoulei") != nullptr) {
            name = "?????????";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "Grenade_Smoke") != nullptr) {
            name = "?????????";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "Grenade_Burn") != nullptr) {
            name = "?????????";
            *color = missile_color;
            return name;
        }
        if (strcmp(gname, "BP_Grenade_Weapon_Wrapper_Thermite_C") == 0) {
            name = "?????????";
            *color = missile_color;
            return name;
        }
        //???
        if (strcmp(gname, "BP_WEP_Pan_Pickup_C") == 0) {
            name = "?????????";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "Sickle") != nullptr) {
            name = "??????";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "Machete") != nullptr) {
            name = "??????";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "Cowbar") != nullptr) {
            name = "??????";
            *color = missile_color;
            return name;
        }
        if (strstr(gname, "CrossBow") != nullptr) {
            name = "?????????";
            *color = missile_color;
            return name;
        }

    }

    //????????????img
    if (showRifle) {
        if (strcmp(gname, "BP_Rifle_AKM_Wrapper_C") == 0) {
            name = "AKM";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_M416_Wrapper_C") == 0) {
            name = "M416";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_M16A4_Wrapper_C") == 0) {
            name = "M16A4";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_SCAR_Wrapper_C") == 0) {
            name = "SCAR";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_QBZ_Wrapper_C") == 0) {
            name = "QBZ";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_G36_Wrapper_C") == 0) {
            name = "G36C";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_M762_Wrapper_C") == 0) {
            name = "M762";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_Groza_Wrapper_C") == 0) {
            name = "Groza";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_AUG_Wrapper_C") == 0) {
            name = "AUG";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Other_Dp-28_Wrapper_C") == 0) {
            name = "Dp-28";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Other_DP28_Wrapper_C") == 0) {
            name = "DP28";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Other_M249_Wrapper_C") == 0) {
            name = "M249";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_QBU_Wrapper_C") == 0) {
            name = "QBU";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_VAL_Wrapper_C") == 0) {
            name = "AV-VAL";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Other_MG3_Wrapper_C") == 0) {
            name = "MG3";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_HoneyBadger_Wrapper_C") == 0) {
            name = "??????";
            *color = rifle_color;
            return name;
        }
        if (strcmp(gname, "BP_Rifle_Mk47_Wrapper_C") == 0) {
            name = "Mk47";
            *color = rifle_color;
            return name;
        }
    }
    //???????????????
    if (showSubmachine) {
        if (strcmp(gname, "BP_MachineGun_UMP9_Wrapper_C") == 0) {
            name = "UMP9";
            *color = submachine_color;
            return name;
        }
        if (strcmp(gname, "BP_MachineGun_TommyGun_Wrapper_C") == 0) {
            name = "??????????????????";
            *color = submachine_color;
            return name;
        }
        if (strcmp(gname, "BP_MachineGun_PP19_Wrapper_C") == 0) {
            name = "PP19";
            *color = submachine_color;
            return name;
        }
        if (strcmp(gname, "BP_MachineGun_Uzi_Wrapper_C") == 0) {
            name = "Uzi";
            *color = submachine_color;
            return name;
        }
        if (strcmp(gname, "BP_MachineGun_Vector_Wrapper_C") == 0) {
            name = "Vector";
            *color = submachine_color;
            return name;
        }
        if (strstr(gname, "P90") != nullptr) {
            name = "P90";
            *color = submachine_color;
            return name;
        }
        if (strcmp(gname, "BP_MachineGun_MP5K_Wrapper_C") == 0) {
            name = "MP5K";
            *color = submachine_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_VSS_Wrapper_C") == 0) {
            name = "VSS";
            *color = submachine_color;
            return name;
        }
    }
    //???????????????
    if (showSniper) {

        if (strcmp(gname, "BP_Sniper_Kar98k_Wrapper_C") == 0) {
            name = "Kar98k";
            *color = snipe_color;
            return name;
        }
        if (strstr(gname, "Mosin") != nullptr) {
            name = "????????????";
            *color = snipe_color;
            return name;
        }

        if (strcmp(gname, "BP_Sniper_Mini14_Wrapper_C") == 0) {
            name = "Mini14";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_SKS_Wrapper_C") == 0) {
            name = "SKS";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_M24_Wrapper_C") == 0) {
            name = "M24";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_rifle_Mk47_Wrapper_C") == 0) {
            name = "Mk47";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_WEP_Mk14_Pickup_C") == 0) {
            name = "MK14";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_AWM_Wrapper_C") == 0) {
            name = "AWM";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_SLR_Wrapper_C") == 0) {
            name = "SLR";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Other_CrossbowBorderland_Wrapper_C") == 0) {
            name = "?????????";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_MK12_Wrapper_C") == 0) {
            name = "MK12";
            *color = snipe_color;
            return name;
        }
        if (strstr(gname, "AMR") != nullptr) {
            name = "AMR";
            *color = snipe_color;
            return name;
        }
        if (strcmp(gname, "BP_Sniper_Win94_Wrapper_C") == 0) {
            name = "Win94";
            *color = snipe_color;
            return name;
        }
    }
    //??????
    if (showMirror) {
        if (strstr(gname, "MZJ_8X") != nullptr) {
            name = "8??????";
            *color = mirror_color;
            return name;
        }
        if (strstr(gname, "MZJ_6X") != nullptr) {
            name = "6??????";
            *color = mirror_color;
            return name;
        }
        if (strstr(gname, "MZJ_4X") != nullptr) {
            name = "4??????";
            *color = mirror_color;
            return name;
        }
        if (strstr(gname, "MZJ_3X") != nullptr) {
            name = "3??????";
            *color = mirror_color;
            return name;
        }
        if (strstr(gname, "MZJ_2X") != nullptr) {
            name = "2??????";
            *color = mirror_color;
            return name;
        }
        if (strstr(gname, "MZJ_HD") != nullptr) {
            name = "???????????????";
            *color = otherparts_color;
            return name;
        }
        if (strstr(gname, "MZJ_QX") != nullptr) {
            name = "???????????????";
            *color = otherparts_color;
            return name;
        }
    }
    //??????
    if (showExpansion) {
        if (strcmp(gname, "BP_DJ_Large_EQ_Pickup_C") == 0) {
            name = "????????????";
            *color = expansion_color;
            return name;
        }
        if (strcmp(gname, "BP_DJ_Large_E_Pickup_C") == 0) {
            name = "????????????";
            *color = expansion_color;
            return name;
        }
        if (strcmp(gname, "BP_DJ_Sniper_EQ_Pickup_C") == 0) {
            name = "???????????????";
            *color = mirror_color;
            return name;
        }
        if (strcmp(gname, "BP_DJ_Sniper_E_Pickup_C") == 0) {
            name = "???????????????";
            *color = mirror_color;
            return name;
        }
        if (strcmp(gname, "BP_DJ_Mid_E_Pickup_C") == 0) {
            name = "???????????????";
            *color = mirror_color;
            return name;
        }
        if (strcmp(gname, "BP_DJ_Mid_EQ_Pickup_C") == 0) {
            name = "???????????????";
            *color = mirror_color;
            return name;
        }
    }
    //????????????
    if (showOtherParts) {
        if (strcmp(gname, "BP_QK_Large_Suppressor_Pickup_C") == 0) {
            name = "????????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QK_Sniper_Suppressor_Pickup_C") == 0) {
            name = "???????????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QT_Sniper_Pickup_C") == 0) {
            name = "??????????????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_ZDD_Sniper_Pickup_C") == 0) {
            name = "??????????????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QK_Large_Compensator_Pickup_C") == 0) {
            name = "??????????????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QK_Sniper_Compensator_Pickup_C") == 0) {
            name = "??????????????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_WB_Vertical_Pickup_C") == 0) {
            name = "??????????????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QT_A_Pickup_C") == 0) {
            name = "????????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_WB_Angled_Pickup_C") == 0) {
            name = "??????????????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_WB_ThumbGrip_Pickup_C") == 0) {
            name = "??????????????????";
            *color = otherparts_color;
            return name;
        }
        if (strstr(gname, "Ghillie")) {
            name = "?????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QK_DuckBill_Pickup_C") == 0) {
            name = "???????????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_QK_Choke_Pickup_C") == 0) {
            name = "?????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_ZDD_Crossbow_Q_Pickup_C") == 0) {
            name = "??????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_DJ_ShotGun_Pickup_C") == 0) {
            name = "??????????????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_WB_LightGrip_Pickup_C") == 0) {
            name = "????????????";
            *color = otherparts_color;
            return name;
        }
        if (strcmp(gname, "BP_WB_HalfGrip_Pickup_C") == 0) {
            name = "???????????????";
            *color = otherparts_color;
            return name;
        }if (strcmp(gname, "BP_WB_Lasersight_Pickup_C") == 0) {
            name = "???????????????";
            *color = otherparts_color;
            return name;
        }
    }
    //????????????
    if (showDrug) {
        if (strstr(gname, "Injection")) {
            name = "????????????";
            *color = drug_color;
            return name;
        }
        if (strstr(gname, "Firstaid")) {
            name = "?????????";
            *color = drug_color;
            return name;
        }
        if (strstr(gname, "FirstAidbox")) {
            name = "?????????";
            *color = drug_color;
            return name;
        }
        if (strstr(gname, "Pills")) {
            name = "?????????";
            *color = drug_color;
            return name;
        }
        if (strstr(gname, "Drink")) {
            name = "????????????";
            *color = drug_color;
            return name;
        }
        if (strstr(gname, "Bandage")) {
            name = "????????????";
            *color = drug_color;
            return name;
        }
    }
    //????????????
    if (showArmor) {
        if (strstr(gname, "Helmet_Lv3")) {
            name = "?????????";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Armor_Lv3")) {
            name = "?????????";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Bag_Lv3")) {
            name = "?????????";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Helmet_Lv2")) {
            name = "?????????";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Armor_Lv2")) {
            name = "?????????";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Bag_Lv2")) {
            name = "?????????";
            *color = armor_color;
            return name;
        }
        /*if (strstr(gname, "Helmet_Lv1")) {
            name = "?????????";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Armor_Lv1")) {
            name = "?????????";
            *color = armor_color;
            return name;
        }
        if (strstr(gname, "Bag_Lv1")) {
            name = "?????????";
            *color = armor_color;
            return name;
        }*/
    }

    //??????????????????
    if (showSpecial) {
        if (strstri(gname, "AssaultSoldier")) {
            name = "?????????_????????????";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "SupplySoldier")) {
            name = "?????????_????????????";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "EngineerSoldier")) {
            name = "?????????_????????????";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "AgileSoldier")) {
            name = "?????????_????????????";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "MedicSoldier")) {
            name = "?????????_????????????";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "CoreRou")) {
            name = "??????????????????";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "CoreTri")) {
            name = "??????????????????";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "CoreSqu")) {
            name = "??????????????????";
            *color = special_color;
            return name;
        }

        if (strstri(gname, "waiguge")) {
            name = "???????????????";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "SparLv1")) {
            name = "????????????";
            *color = special_color;
            return name;
        }
        if (strstri(gname, "ResurrectionCard")) {
            name = "????????????";
            *color = special_color;
            return name;
        }

    }

    if (showOther) {
        if (strstri(gname, "GasCanBattery_Destructible")) {
            name = "??????";
            *color = other_color;
            return name;
        }
        if (strstri(gname, "EntranceFactoryCard")) {
            name = "3????????????";
            *color = other_color;
            return name;
        }
        if (strstri(gname, "FlareGun")) {
            name = "?????????";
            *color = other_color;
            return name;
        }
        if (strstri(gname, "BP_revivalAED_Pickup_C")) {
            name = "?????????";
            *color = other_color;
            return name;
        }
        if (strstri(gname, "HumanCannon_Pickup_C")) {
            name = "?????????";
            *color = other_color;
            return name;
        }
        if (strstri(gname, "PickUp_BP_Partner")) {
            name = "??????";
            *color = other_color;
            return name;
        }
    }

    return "";
}

//????????????
string GetVehicleInfo(const char *gname, ImVec4 *color) {
    string name;
    if (strcmp(gname, "VH_BRDM_C") == 0) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }
    if (strcmp(gname, "VH_Scooter_C") == 0) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }
    if (strcmp(gname, "VH_Motorcycle_C") == 0 || strcmp(gname, "VH_Motorcycle_1_C") == 0) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }
    if (strcmp(gname, "VH_MotorcycleCart_1_C") == 0 || strcmp(gname, "VH_MotorcycleCart_C") == 0) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }
    if (strcmp(gname, "VH_Snowmobile_C") == 0) {
        name = "????????????";
        *color = vehicle_color;
        return name;
    }
    if (strcmp(gname, "BP_VH_Tuk_C") == 0 || strcmp(gname, "BP_VH_Tuk_1_C") == 0) {
        name = "????????????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "Buggy")) {
        name = "??????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "Dacia")) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "UAZ")) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "_PickUp")) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "Rony")) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "Mirado")) {
        name = "??????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "MiniBus")) {
        name = "????????????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "PG117")) {
        name = "??????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "AquaRail")) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }

    if (strstri(gname, "AquaRail")) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "AirDropPlane")) {
        name = "????????????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "BP_AirDropPlane_C")) {
        name = "???????????????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "Bigfoot")) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "ATV")) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "CoupeRB")) {
        name = "??????";
        *color = vehicle_color;
        return name;
    }
    if (strstri(gname, "BP_SciFi_Moto_C")) {
        name = "?????????";
        *color = vehicle_color;
        return name;
    }

    return "";
}

//????????????
string getWeapon(int id) {
    if (id == 101006) {
        return "AUG";
    } else if (id == 101008) {
        return "M762";
    } else if (id == 101003) {
        return "SCAR-L";
    } else if (id == 101004) {
        return "M416";
    } else if (id == 101004) {
        return "M16A-4";
    } else if (id == 101009) {
        return "Mk47";
    } else if (id == 101010) {
        return "G36C";
    } else if (id == 101007) {
        return "QBZ";
    } else if (id == 101001) {
        return "AKM";
    } else if (id == 101005) {
        return "Groza";
    } else if (id == 102005) {
        return "??????";
    } else if (id == 102004) {
        return "?????????";
    } else if (id == 102007) {
        return "MP5K";
    } else if (id == 102002) {
        return "UMP45";
    } else if (id == 102003) {
        return "Vector";
    } else if (id == 102001) {
        return "Uzi";
    } else if (id == 105002) {
        return "DP28";
    } else if (id == 105001) {
        return "M249";
    } else if (id == 103001) {
        return "Kar98k";
    } else if (id == 103002) {
        return "M24";
    } else if (id == 103003) {
        return "AWM";
    } else if (id == 103010) {
        return "QBU";
    } else if (id == 103009) {
        return "SLR";
    } else if (id == 103004) {
        return "SKS";
    } else if (id == 103006) {
        return "Mini14";
    } else if (id == 103005) {
        return "VSS";
    } else if (id == 103008) {
        return "Win94";
    } else if (id == 103007) {
        return "Mk14";
    } else if (id == 103903) {
        return "????????????";
    } else if (id == 104003) {
        return "S12K";
    } else if (id == 104004) {
        return "DBS";
    } else if (id == 104001) {
        return "S686";
    } else if (id == 104002) {
        return "S1897";
    } else if (id == 108003) {
        return "??????";
    } else if (id == 108001) {
        return "?????????";
    } else if (id == 108002) {
        return "??????";
    } else if (id == 107001) {
        return "?????????";
    } else if (id == 108004) {
        return "?????????";
    } else if (id == 106006) {
        return "????????????";
    } else if (id == 106003) {
        return "R1895";
    } else if (id == 106008) {
        return "Vz61";
    } else if (id == 106001) {
        return "P92";
    } else if (id == 106004) {
        return "P18C";
    } else if (id == 106005) {
        return "R45";
    } else if (id == 106002) {
        return "P1911";
    } else if (id == 106010) {
        return "????????????";
    } else if (id == 106107) {
        return "????????????";
    } else if (id == 101002) {
        return "M16A4";
    } else if (id == 101011) {
        return "AC-VAL";
    } else if (id == 102105) {
        return "P90";
    } else if (id == 103012) {
        return "AMR";
    } else if (id == 103013) {
        return "M417";
    } else if (id == 103100) {
        return "Mk12";
    } else if (id == 103901) {
        return "Kar98K";
    } else if (id == 103902) {
        return "M24";
    } else if (id == 103903) {
        return "AWM";
    } else if (id == 103903) {
        return "SPAS-12";
    } else if (id == 105002) {
        return "DP-28";
    } else if (id == 105003) {
        return "M134";
    } else if (id == 105010) {
        return "MG3";
    } else if (id == 107986) {
        return "Kar98K";
    } else if (id == 602001) {
        return "?????????";
    } else if (id == 602002) {
        return "?????????";
    } else if (id == 602003) {
        return "?????????";
    } else if (id == 602004) {
        return "???????????????";
    } else if (id == 0) {
        return "??????";
    } else if (id == 101012) {
        return "??????";
    } else {
        return "??????("+ to_string(id) +")";
    }
}