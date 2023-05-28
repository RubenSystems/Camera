import os,sys

class ObjectTracker(object):
    def __init__(self, trackerObjectName):
        if trackerObjectName == 'sort':  # Add more trackers in elif whenever needed
            self.trackerObject = SortTracker()
        else:
            print("Invalid Tracker Name")
            self.trackerObject = None


class SortTracker(ObjectTracker):
    def __init__(self):
        sys.path.append(os.path.join(os.path.dirname(__file__), '../third_party', 'sort-master'))
        from sort import Sort
        self.mot_tracker = Sort()


ObjectTracker("sort")
