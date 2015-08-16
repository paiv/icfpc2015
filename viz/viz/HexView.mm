//
//  HexView.m
//  viz
//
//  Created by Pavel Ivashkov on 2015-08-08.
//  Copyright (c) 2015 paiv. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import "HexView.h"

#include <array>
#include <memory>
#include "point.hpp"
#include "paiv.hpp"
#include "json.h"
#include "hexagons_platform.hpp"
#include "../../code/src/json_reader.cpp"
#include "../../code/src/game.cpp"
#include "../../code/src/path_finder.cpp"
#include "../../code/src/solve_phrases.cpp"
#include "../../code/src/random_player.cpp"
#include "../../code/src/mc_player.cpp"
#include "../../code/src/placement_player.cpp"

using namespace std;
using namespace paiv;

@interface HexView ()
{
    string problemFileName;
    problem _problem;
    game_state currentState;
    random_player randomPlayer;
    placement_player monteCarloPlayer;
}

@property BOOL abortPlayer;
@property SystemSoundID soundId;

@property NSColor *gridColor;
@property NSColor *gridFill;
@property NSColor *filledColor;
@property NSColor *unitFill;
@property NSColor *unitPivot;
@property NSColor *legendColor;
@property NSColor *legendFill;
@property NSColor *legendPivot;
@property NSColor *candidateColor;

@end

@implementation HexView

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];

    [self setupMenu];
    
    self.gridColor = [NSColor lightGrayColor];
    self.gridFill = [NSColor clearColor];
    self.filledColor = [NSColor colorWithRed:13/256.0 green:13/256.0 blue:53/256.0 alpha:1];
    self.unitFill = [NSColor colorWithRed:13/256.0 green:53/256.0 blue:213/256.0 alpha:1];
    self.unitPivot = [NSColor lightGrayColor];
    self.legendColor = [NSColor darkGrayColor];
    self.legendFill = [NSColor clearColor];
    self.legendPivot = [NSColor redColor];
    self.candidateColor = [NSColor redColor];

    NSURL *clickWav = [[NSBundle mainBundle] URLForResource:@"hit.wav" withExtension:nil];
    AudioServicesCreateSystemSoundID((__bridge CFURLRef)clickWav, &_soundId);

    
    problemFileName = "/Users/pasha/Documents/paiv/icfpc_2015/spec/problems/problem_0.json";
    [self setupGame];
    
    return self;
}

- (void)dealloc
{
    AudioServicesDisposeSystemSoundID(self.soundId);
}

- (void)setupMenu
{
    NSMenu *menu = [[NSMenu alloc] initWithTitle:@"Play"];
    [menu addItemWithTitle:@"Reload" action:@selector(handlePlayReload:) keyEquivalent:@""];
    [menu addItem:[NSMenuItem separatorItem]];
    [menu addItemWithTitle:@"Random Player" action:@selector(handlePlayRandomPlayer:) keyEquivalent:@""];
    [menu addItemWithTitle:@"Monte-Carlo Player" action:@selector(handlePlayMonteCarloPlayer:) keyEquivalent:@""];
    [menu addItem:[NSMenuItem separatorItem]];
    [menu addItemWithTitle:@"Load State" action:@selector(handlePlayLoadState:) keyEquivalent:@""];
    [menu addItemWithTitle:@"One Step" action:@selector(handlePlayOneStep:) keyEquivalent:@""];
    [menu addItem:[NSMenuItem separatorItem]];
    [menu addItemWithTitle:@"Stop It!" action:@selector(handlePlayAbortPlayer:) keyEquivalent:@""];
    [self setMenu:menu];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)handlePlayReload:(NSEvent *)event
{
    [self setupGame];
}

- (void)handlePlayRandomPlayer:(NSEvent *)event
{
    self.abortPlayer = NO;
    [self scheduleRandomPlayerStep];
}

- (void)handlePlayMonteCarloPlayer:(NSEvent *)event
{
    self.abortPlayer = NO;
    [self scheduleMCPlayerStep];
}

- (void)handlePlayLoadState:(NSEvent *)event
{
    currentState = GameLoadState(3, _problem, 0,
" |         o           |"
" |          o          |"
" |         o           |"
" |                     |"
" |                     |"
" |                     |"
" |                     |"
" |                     |"
" |                     |"
" |      o       o      |"
                                 );
    [self setNeedsDisplay:YES];
}

- (void)handlePlayOneStep:(NSEvent *)event
{
    self.abortPlayer = YES;
    [self scheduleMCPlayerStep];
}

- (void)handlePlayAbortPlayer:(NSEvent *)event
{
    self.abortPlayer = YES;
}

- (void)keyDown:(NSEvent *)theEvent
{
}

- (void)keyUp:(NSEvent *)theEvent
{
    if (currentState.is_terminal)
        return;

    moved action;
    
    switch (theEvent.keyCode)
    {
        case 123:
            action = moved::W;
            break;
        case 124:
            action = moved::E;
            break;
        case 125:
            action = moved::CW;
            break;
        case 126:
            action = moved::CCW;
            break;
            
        default:
            switch ([theEvent.characters characterAtIndex:0])
            {
                case 'q':
                    action = moved::W;
                    break;
                case 'w':
                    action = moved::CCW;
                    break;
                case 'e':
                    action = moved::E;
                    break;
                case 'a':
                    action = moved::SW;
                    break;
                case 's':
                    action = moved::CW;
                    break;
                case 'd':
                    action = moved::SE;
                    break;
                    
                case 't':
                    [self handlePlayOneStep:theEvent];
                    return;
                    
                default:
                    return;
            }
    }

    AudioServicesPlaySystemSound(self.soundId);
    
    [self gamePlayAction:action];
}

- (void)gamePlayAction:(moved)action
{
    currentState = GameGetNextState(currentState, action);
    
    [self setNeedsDisplay:YES];

    // clog << "      step: " << currentState.unitPosition << endl;

    if (currentState.is_terminal)
    {
        self.abortPlayer = YES;
        NSAlert *alert = [[NSAlert alloc] init];
        alert.messageText = @"GAME OVER";
        alert.informativeText = [NSString stringWithFormat:@"Score: %d (%d units left)", currentState.score, currentState.unitsLeft];
        [alert runModal];
    }
}

- (void)setupGame
{
    _problem = read_problem(problemFileName);
    currentState = GameGetInitialState(_problem, _problem.source_seeds.front());
    [self setNeedsDisplay:YES];
}

- (void)scheduleRandomPlayerStep
{
    [NSTimer scheduledTimerWithTimeInterval:0.2 target:self selector:@selector(randomPlayerDoStep:) userInfo:nil repeats:NO];
}

- (void)scheduleMCPlayerStep
{
    [NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(monteCarloPlayerDoStep:) userInfo:nil repeats:NO];
}

- (void)randomPlayerDoStep:(NSTimer *)timer
{
    moved action = randomPlayer.step(currentState);
//    NSLog(@"PLAY %c", (char)action);
    [self gamePlayAction:action];
    if (!self.abortPlayer)
        [self scheduleRandomPlayerStep];
}

- (void)monteCarloPlayerDoStep:(NSTimer *)timer
{
    moved action = monteCarloPlayer.step(currentState);
//    NSLog(@"PLAY %c", (char)action);
    [self gamePlayAction:action];
//    cout << "current pos: " << currentState.unitPosition << endl;
    if (!self.abortPlayer)
        [self scheduleMCPlayerStep];
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    CGFloat h = CGRectGetHeight(self.bounds);
    [NSGraphicsContext saveGraphicsState];
    
    NSAffineTransform *tr = [NSAffineTransform transform];
    [tr scaleXBy:1 yBy:-1];
    [tr translateXBy:0 yBy:-h];
    [tr concat];

    [self drawGameState:currentState];

    [NSGraphicsContext restoreGraphicsState];
}

- (void)drawGameState:(game_state&)state
{
    Layout lyt(layout_pointy, NSMakePoint(10,10), NSMakePoint(40, 40));
    Layout lyt2(layout_pointy, NSMakePoint(4,4), NSMakePoint(_problem.width * 20 + 48, 40));

    [self drawBoard:lyt state:state];
    if (state.unitIndex >= 0)
        [self drawCurrentUnit:state.problem->units[state.unitIndex] placed:state.unitPosition layout:lyt];
    
    // [self drawLegend:lyt2];
    
    [self drawNextUnits:GameGetNextUnits(5, state) layout:lyt2];
    
//    unit& u = state.problem->units[state.unitIndex];
//    list<placement> places = GameGetCandidatePlacements(state);
//    [self drawCandidatePlacements:places unit:u layout:lyt];
}

- (void)drawBoard:(Layout&)lyt state:(game_state&)state
{
    [self.gridColor setStroke];
    [self.gridFill setFill];
    
    for (s16 row = 0; row < _problem.height; row++)
    {
        for (s16 col = 0; col < _problem.width; col++)
            [self drawTile:roffset_to_cube(0, OffsetCoord {col, row}) layout:lyt lineWidth:1];
    }
    
    [self.filledColor setFill];
    
    board& board = state.board;
    for (s16 row = 0; row < _problem.height; row++)
    {
        for (s16 col = 0; col < _problem.width; col++)
        {
            if (board.at(col, row))
                [self drawTile:roffset_to_cube(0, {col, row}) layout:lyt lineWidth:2];
        }
    }
}

- (void)drawCandidatePlacements:(list<placement>&)places unit:(unit&)u layout:(Layout&)lyt
{
    [self.candidateColor setStroke];
    [self.gridFill setFill];
    
    int h = 0;
    for (auto& pos : places)
    {
        unit movedUnit = GameMoveUnit(u, pos);
        
        [[NSColor colorWithHue:(h / 100.) saturation:1 brightness:1 alpha:1] setStroke];
        
        [self drawUnit:movedUnit layout:lyt];
        
        h += 15;
        if (h > 100)
            h -= 100;
    }
}

- (void)drawNextUnits:(const list<unit>&)units layout:(Layout&)lyt
{
    [self.legendFill setFill];

    s16 yoffset = 0;
    for (const unit& u : units)
    {
        auto x = roffset_to_cube(0, {0, yoffset});
        placement pl = {x.q, x.r, angle::R0};
        unit v = GameMoveUnit(u, pl);
        
        [self.legendColor setStroke];
        [self drawUnit:v layout:lyt];
        
        [self.legendPivot setStroke];
        [self drawPivot:v.pivot layout:lyt size:CGSizeMake(2, 2)];

        yoffset += unit_size(u).r + 2;
}
}

- (void)drawLegend:(Layout&)lyt
{
    [self.legendFill setFill];
    
    s16 yoffset = 0;
    s16 xoffset = 0;
    for (unit& u : _problem.units)
    {
        s16 r = yoffset;
        s16 q = xoffset + (yoffset / 2 % 4) * 4;
        
        xoffset--;
        yoffset += 2;

        placement pl = {q,r,angle::R0};
        unit v = GameMoveUnit(u, pl);
        
        [self.legendColor setStroke];
        [self drawUnit:v layout:lyt];
        
        [self.legendPivot setStroke];
        [self drawPivot:v.pivot layout:lyt size:CGSizeMake(2, 2)];
    }
}

- (void)drawCurrentUnit:(unit)u placed:(placement&)pl layout:(Layout&)lyt
{
    unit v = GameMoveUnit(u, pl);

    [self.unitFill setFill];
    [self drawUnit:v layout:lyt];
    
    [self.unitPivot setStroke];
    [self drawPivot:v.pivot layout:lyt size:CGSizeMake(2, 2)];
}

- (void)drawUnit:(unit&)u layout:(Layout&)lyt
{
    for (cell& c : u.members)
        [self drawTile:c layout:lyt lineWidth:1];
}

- (void)drawPivot:(cell&)c layout:(Layout&)lyt size:(CGSize)sz
{
    paiv::Point p = hex_to_pixel(lyt, c);
    p.x -= 1;
    p.y -= 1;
    [self dropCircleAt:p size:CGSizeMake(sz.width, sz.height)];
}

- (void)dropCircleAt:(CGPoint)point size:(CGSize)sz
{
    NSRect bounds = NSMakeRect(point.x, point.y, sz.width, sz.height);
    
    NSBezierPath *circle = [NSBezierPath bezierPathWithOvalInRect:bounds];
    [circle setLineWidth:2];
    [circle stroke];
}

- (void)drawTile:(Hex)hex layout:(Layout&)lyt lineWidth:(CGFloat)width
{
    
    NSBezierPath *path = [NSBezierPath bezierPath];
    vector<paiv::Point> pts = polygon_corners(lyt, hex);
    
    [path moveToPoint:pts.front()];
    
    for (int i = 1; i < 7; i++)
        [path lineToPoint:pts[i % 6]];
    
    if (width > 0)
    {
        [path setLineWidth:width];
        [path stroke];
    }
    
    [path fill];
}

@end
