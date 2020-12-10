from validate import LCausalBroadcastValidation

tests = [{
    'outputDir': 'test_outputs/t1',
    'processes': 5,
    'messages': 20,
    'dependencies': {
        1: [1],
        2: [2],
        3: [3, 1, 2, 4, 5],
        4: [4, 1, 5],
        5: [5, 1, 2, 3, 4] 
    }
}]

def test_lcb():
    
    for test in tests:
        print(test)
        validation = LCausalBroadcastValidation(test['processes'], test['messages'], test['outputDir'],test['dependencies'])
        assert(validation.checkAll() == True) 

