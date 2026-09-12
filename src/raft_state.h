#pragma once

#include <string>
#include <mutex>

enum class NodeRole {
    FOLLOWER,
    CANDIDATE,
    LEADER
};

class RaftState {
public:
    RaftState(int nodeId);

    // Term and role management
    int getCurrentTerm();
    NodeRole getRole();
    int getNodeId() const;

    // Called when this node sees a higher term from someone else —
    // per Raft's core rule, it must step down and update its term.
    void stepDownToFollower(int newTerm);

    // Transition helpers
    void becomeCandidate();   // increments term, votes for self
    void becomeLeader();
    void becomeFollower();

    // Voting
    bool tryGrantVote(int candidateTerm, int candidateId);

private:
    std::mutex mutex_;
    int nodeId_;
    int currentTerm_;
    NodeRole role_;
    int votedFor_;   // -1 if hasn't voted this term
};