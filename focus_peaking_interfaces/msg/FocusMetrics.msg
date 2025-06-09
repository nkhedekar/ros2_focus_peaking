# FocusMetrics.msg

# Header to include timestamp
std_msgs/Header header

# The current calculated focus score
float64 current_score

# The minimum focus score observed in the recent history
float64 min_score

# The maximum focus score observed in the recent history
float64 max_score

# window size for focus score history
int32 window_size