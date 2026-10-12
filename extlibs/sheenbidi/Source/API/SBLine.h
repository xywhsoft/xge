/* XGE portability: project includes use relative paths. */
/*
 * Copyright (C) 2014-2025 Muhammad Tayyab Akram
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _SB_INTERNAL_LINE_H
#define _SB_INTERNAL_LINE_H

#include "../../Headers/SheenBidi/SBCodepointSequence.h"
#include "../../Headers/SheenBidi/SBLine.h"
#include "../../Headers/SheenBidi/SBParagraph.h"
#include "../../Headers/SheenBidi/SBRun.h"

#include "SBBase.h"
#include "../Core/Object.h"

typedef struct _SBLine {
    ObjectBase _base;
    SBCodepointSequence codepointSequence;
    SBRun *fixedRuns;
    SBUInteger runCount;
    SBUInteger offset;
    SBUInteger length;
} SBLine;

SB_INTERNAL SBLineRef SBLineCreate(SBParagraphRef paragraph,
    SBUInteger lineOffset, SBUInteger lineLength);

#endif
