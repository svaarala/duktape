function Run() {
  BenchmarkSuite.RunSuites({ NotifyStep: ShowProgress,
                             NotifyError: AddError,
                             NotifyResult: AddResult,
                             NotifyScore: AddScore });
}

function ShowProgress(name) {
  print('PROGRESS', name);
}

function AddError(name, error) {
  print('ERROR', name, error);
  print(error.stack);
}

function AddResult(name, result) {
  print('RESULT', name, result);
}

function AddScore(score) {
  print('SCORE', score);
}

try {
  Run();
} catch (e) {
  print('*** Run() failed');
  print(e.stack || e);
}
