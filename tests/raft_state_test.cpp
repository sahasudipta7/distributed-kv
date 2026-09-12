#include <gtest/gtest.h>
#include "../src/raft_state.h"

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