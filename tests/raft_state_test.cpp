#include <gtest/gtest.h>
#include "../src/raft_state.h"

#include "../src/election_timer.h"
#include <iostream>
#include <chrono>

#include "../src/raft_node.h"

TEST(RaftNodeTest, TimeoutTriggersCandidateTransition) {
    RaftNode node(1);

    ASSERT_EQ(node.getState().getRole(), NodeRole::FOLLOWER);
    ASSERT_EQ(node.getState().getCurrentTerm(), 0);

    node.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(400));  // wait past max timeout
    node.stop();

    ASSERT_EQ(node.getState().getRole(), NodeRole::CANDIDATE);
    ASSERT_GE(node.getState().getCurrentTerm(), 1);
}

TEST(ElectionTimerTest, FiresAfterTimeout) {
    std::atomic<bool> fired(false);

    ElectionTimer timer(100, 150, [&fired]() {
        fired = true;
    });

    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    timer.stop();

    ASSERT_TRUE(fired);
}

TEST(ElectionTimerTest, ResetPreventsTimeout) {
    std::atomic<int> fireCount(0);

    ElectionTimer timer(100, 150, [&fireCount]() {
        fireCount++;
    });

    timer.start();

    // Keep resetting faster than the timeout, for 300ms total
    for (int i = 0; i < 6; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        timer.reset();
    }

    timer.stop();

    ASSERT_EQ(fireCount, 0);  // should never have fired, we kept resetting it
}

TEST(RaftStateTest, StartsAsFollowerWithTermZero) {
    RaftState state(1);
    ASSERT_EQ(state.getRole(), NodeRole::FOLLOWER);
    ASSERT_EQ(state.getCurrentTerm(), 0);
}

TEST(RaftStateTest, BecomingCandidateIncrementsTermAndVotesForSelf) {
    RaftState state(1);
    state.becomeCandidate();
    ASSERT_EQ(state.getRole(), NodeRole::CANDIDATE);
    ASSERT_EQ(state.getCurrentTerm(), 1);
}

TEST(RaftStateTest, GrantsVoteToHigherTermCandidate) {
    RaftState state(1);
    bool granted = state.tryGrantVote(5, 2);
    ASSERT_TRUE(granted);
    ASSERT_EQ(state.getCurrentTerm(), 5);
}

TEST(RaftStateTest, RejectsVoteFromLowerTermCandidate) {
    RaftState state(1);
    state.stepDownToFollower(10);  // simulate this node already being at term 10
    bool granted = state.tryGrantVote(5, 2);
    ASSERT_FALSE(granted);
    ASSERT_EQ(state.getCurrentTerm(), 10);  // term unchanged
}

TEST(RaftStateTest, DoesNotVoteTwiceInSameTerm) {
    RaftState state(1);
    bool firstVote = state.tryGrantVote(3, 2);   // votes for node 2
    bool secondVote = state.tryGrantVote(3, 4);  // node 4 asks in same term

    ASSERT_TRUE(firstVote);
    ASSERT_FALSE(secondVote);  // already committed to node 2 this term
}

TEST(RaftStateTest, CanVoteAgainForSameCandidateSameTerm) {
    RaftState state(1);
    bool firstVote = state.tryGrantVote(3, 2);
    bool secondVote = state.tryGrantVote(3, 2);  // duplicate request, same candidate

    ASSERT_TRUE(firstVote);
    ASSERT_TRUE(secondVote);
}

TEST(RaftStateTest, StepDownResetsVoteAndUpdatesTerm) {
    RaftState state(1);
    state.tryGrantVote(3, 2);
    state.stepDownToFollower(7);
    ASSERT_EQ(state.getCurrentTerm(), 7);
    ASSERT_EQ(state.getRole(), NodeRole::FOLLOWER);

    // Should be able to vote again since term changed
    bool granted = state.tryGrantVote(7, 5);
    ASSERT_TRUE(granted);
}