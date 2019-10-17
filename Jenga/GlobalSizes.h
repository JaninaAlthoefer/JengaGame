#pragma once

const int numBlocks = 54;

const float halfBoxLongSide = 1.0f;
const float halfBoxShortSide = halfBoxLongSide * 0.2f;
const float halfBoxMiddleSide = halfBoxLongSide / 3.0f;

enum blockLayout {LEFT, MIDDLE, RIGHT, LEFTMIDDLE, RIGHTMIDDLE, LEFTRIGHT, FULL};